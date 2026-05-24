#include <Clients/RigidBody.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <AzCore/std/utility/as_const.h>
#include <AzCore/Math/MathStringConversions.h>
#include <AzFramework/Physics/Utils.h>
#include <AzFramework/Physics/Configuration/RigidBodyConfiguration.h>

#include <Jolt/Jolt.h>
#include "Jolt/Physics/PhysicsSystem.h"
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Collision/Shape/EmptyShape.h"

#include <Utils.h>
#include <Clients/Shape.h>
#include <System/CollisionLayerFilters.h>
#include <JoltPhysics/Utils.h>
#include <JoltPhysics/MathConversions.h>
#include <JoltPhysics/NativeTypeIdentifiers.h>

#include "Jolt/Physics/Body/BodyLock.h"
#include "Jolt/Physics/Collision/Shape/EmptyShape.h"
#include "Jolt/Physics/Collision/Shape/StaticCompoundShape.h"

namespace JoltPhysics
{
    void RigidBodyConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<JoltPhysics::RigidBodyConfiguration>()
                ->Version(1)
                ->Field("CollisionConfiguration", &JoltPhysics::RigidBodyConfiguration::m_colliderConfig)
                ->Field("SolverVelocityIterations", &JoltPhysics::RigidBodyConfiguration::m_solverVelocityIterations)
                ->Field("SolverPositionIterations", &JoltPhysics::RigidBodyConfiguration::m_solverPositionIterations)
                ->Field("CanSleep", &JoltPhysics::RigidBodyConfiguration::m_canSleep);

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<JoltPhysics::RigidBodyConfiguration>("JoltPhysics-specific Rigid Body Configuration",
                    "Additional Rigid Body settings specific to JoltPhysics.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &RigidBodyConfiguration::m_colliderConfig, 
                        "Collision Configuration", 
                        "Collision configuration for the body.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &JoltPhysics::RigidBodyConfiguration::m_solverVelocityIterations,
                        "Solver Velocity Iterations",
                        "Higher values can improve stability at the cost of performance. 0 follows default in Jolt configuration")
                        ->Attribute(AZ::Edit::Attributes::Min, 0)
                        ->Attribute(AZ::Edit::Attributes::Max, 255)
                        ->Attribute(AZ::Edit::Attributes::Visibility, &RigidBodyConfiguration::GetSolverIterationVisibility)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &JoltPhysics::RigidBodyConfiguration::m_solverPositionIterations,
                        "Solver Position Iterations",
                        "Higher values can improve stability at the cost of performance. 0 follows default in Jolt configuration")
                        ->Attribute(AZ::Edit::Attributes::Min, 0)
                        ->Attribute(AZ::Edit::Attributes::Max, 255)
                        ->Attribute(AZ::Edit::Attributes::Visibility, &RigidBodyConfiguration::GetSolverIterationVisibility)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &JoltPhysics::RigidBodyConfiguration::m_canSleep,
                        "Can Sleep",
                        "Whether this body is allowed to sleep ever."
                        )
                        ->Attribute(AZ::Edit::Attributes::Visibility, &RigidBodyConfiguration::GetCanSleepVisibility);
            }
        }
    }

    AZ::Crc32 RigidBodyConfiguration::GetPropertyVisibility(PropertyVisibility property) const
    {
        return (m_propertyVisibilityFlags & property) != 0 ? AZ::Edit::PropertyVisibility::Show : AZ::Edit::PropertyVisibility::Hide;
    }

    void RigidBodyConfiguration::SetPropertyVisibility(PropertyVisibility property, bool isVisible)
    {
        if (isVisible)
        {
            m_propertyVisibilityFlags |= property;
        }
        else
        {
            m_propertyVisibilityFlags &= ~property;
        }
    }

    AZ::Crc32 RigidBodyConfiguration::GetSolverIterationVisibility() const
    {
        return GetPropertyVisibility(PropertyVisibility::SolverIteration);
    }

    AZ::Crc32 RigidBodyConfiguration::GetCanSleepVisibility() const
    {
        return GetPropertyVisibility(PropertyVisibility::CanSleep);
    }

    RigidBody::RigidBody(const AzPhysics::RigidBodyConfiguration& configuration, JPH::PhysicsSystem& owningSystem)
        : m_owningSystem(&owningSystem)
        , m_startAsleep(configuration.m_startAsleep)
    {
        CreateJoltBody(configuration);
    }

    RigidBody::~RigidBody()
    {
        //clean up the attached shapes
        {
            for (auto shape : m_shapes)
            {
                // We may need to mark on m_joltBody that it is to be deleted
                // TODO: Set body shape to EmptyShape again?
                shape->DetachedFromActor();
            }
        }
        m_shapes.clear();

        // Invalidate user data so it sets m_joltRigidBody->userData to nullptr.
        // It's appropriate to do this as m_joltSRigidBody is a shared pointer, and
        // technically it could survive m_actorUserData life's span.
        m_bodyUserData.Invalidate();
    }

    void RigidBody::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RigidBody>()
                ->Version(1)
            ;
        }
    }

    void RigidBody::CreateJoltBody(const AzPhysics::RigidBodyConfiguration& configuration)
    {
        if (m_joltRigidBody != nullptr)
        {
            AZ_Warning("Jolt Rigid Body", false, "Trying to create Jolt rigid actor when it's already created");
            return;
        }
        // We can't crate a Body without a shape. This will be replaced in AddShape()
        JPH::EmptyShapeSettings emptySettings;
        JPH::Shape* emptyShape = emptySettings.Create().Get();

        JPH::EMotionType motionType;
        if (configuration.m_kinematic)
        {
            motionType = JPH::EMotionType::Kinematic;
        }
        else
        {
            motionType = JPH::EMotionType::Dynamic;
        }

        JPH::BodyCreationSettings newBody;
        newBody.SetShape(emptyShape);
        newBody.mPosition = JoltMathConvert(configuration.m_position);
        newBody.mRotation = JoltMathConvert(configuration.m_orientation);
        newBody.mMotionType = motionType;
        newBody.mObjectLayer = 1 << 1; // Placeholder object layer until ApplyJoltConfiguration()
        newBody.mLinearVelocity = JoltMathConvert(configuration.m_initialLinearVelocity);
        newBody.mAngularVelocity = JoltMathConvert(configuration.m_initialAngularVelocity);

        if (!configuration.m_gravityEnabled)
        {
            newBody.mGravityFactor = 0.0f;
        }
        if (configuration.m_computeInertiaTensor)
        {
            newBody.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
            newBody.mMassPropertiesOverride.mMass = configuration.m_mass;
        }
        if (configuration.m_computeMass && configuration.m_computeInertiaTensor)
        {
            newBody.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
            newBody.mMassPropertiesOverride.mMass = configuration.m_mass;
            newBody.mMassPropertiesOverride.mInertia = JoltMathConvert(configuration.m_inertiaTensor);
        }
        if (configuration.IsCcdEnabled())
        {
            // Performs secondary shape cast per update to prevent tunneling in high velocity collisions
            newBody.mMotionQuality = JPH::EMotionQuality::LinearCast;
        }

        m_joltRigidBody = m_owningSystem->GetBodyInterface().CreateBody(newBody);
        // m_owningSystem->GetBodyInterface().AddBody(m_joltRigidBody->GetID(), JPH::EActivation::DontActivate); // TODO: finalize queuing in JoltScene
        
        if (m_joltRigidBody == nullptr)
        {
            AZ_Warning("RigidBody::CreateJoltBody", false, "Jolt Body pointer was null")
        }

        m_bodyUserData = BodyData(m_joltRigidBody);
        m_bodyUserData.SetRigidBody(this);

        m_bodyUserData.SetEntityId(configuration.m_entityId);

        m_debugName = configuration.m_debugName;
    }

    void RigidBody::AddShape(AZStd::shared_ptr<Physics::Shape> shape)
    {
        if (!m_joltRigidBody || !shape)
        {
            return;
        }

        auto joltShape = AZStd::rtti_pointer_cast<JoltPhysics::Shape>(shape);

        if (!joltShape)
        {
            AZ_Error("Jolt Rigid Body", false, "Trying to add a shape of unknown type. Name: %s", GetName().c_str());
            return;
        }

        if (!joltShape->GetNativePointer())
        {
            AZ_Error("Jolt Rigid Body", false, "Trying to add a shape with no valid JPH::Shape. Name: %s", GetName().c_str());
            return;
        }

        if (static_cast<JPH::Shape*>(shape->GetNativePointer())->GetType() == JPH::EShapeType::Mesh && !IsKinematic())
        {
            AZ_Error("Jolt", false, "Cannot use mesh geometry on a dynamic object: %s", GetName().c_str());
            return;
        }

        joltShape->SetInternalPhysicsSystem(m_owningSystem);
        joltShape->AttachedToActor(m_joltRigidBody);
        m_shapes.push_back(joltShape);
    }

    void RigidBody::RemoveShape([[maybe_unused]] AZStd::shared_ptr<Physics::Shape> shape)
    {
        // TODO: trigger reCreateBody
        if (!m_joltRigidBody || !shape)
        {
            return;
        }

        auto joltShape = AZStd::rtti_pointer_cast<JoltPhysics::Shape>(shape);

        if (!joltShape)
        {
            AZ_Error("Jolt Rigid Body", false, "Trying to remove a shape of unknown type. Name: %s", GetName().c_str());
            return;
        }

        const auto found = AZStd::find(m_shapes.begin(), m_shapes.end(), shape);
        if (found == m_shapes.end())
        {
            AZ_Warning("JoltPhysics::RigidBody", false, "Shape has not been attached to this rigid body: %s", GetName().c_str());
            return;
        }

        m_shapes.erase(found);
    }

    // TODO: Determine appropriate mass properties to reflect and set
    void RigidBody::UpdateMassProperties(
        [[maybe_unused]] AzPhysics::MassComputeFlags flags,
        [[maybe_unused]] const AZ::Vector3& centerOfMassOffsetOverride,
        [[maybe_unused]] const AZ::Matrix3x3& inertiaTensorOverride,
        [[maybe_unused]] const float massOverride)
    {
        const bool computeCenterOfMass = AzPhysics::MassComputeFlags::COMPUTE_COM == (flags & AzPhysics::MassComputeFlags::COMPUTE_COM);
        const bool computeInertiaTensor = AzPhysics::MassComputeFlags::COMPUTE_INERTIA == (flags & AzPhysics::MassComputeFlags::COMPUTE_INERTIA);
        const bool computeMass = AzPhysics::MassComputeFlags::COMPUTE_MASS == (flags & AzPhysics::MassComputeFlags::COMPUTE_MASS);
        const bool needsCompute = computeCenterOfMass || computeInertiaTensor || computeMass;
        AZ_UNUSED(needsCompute);
        const bool includeAllShapesInMassCalculation = AzPhysics::MassComputeFlags::INCLUDE_ALL_SHAPES == (flags & AzPhysics::MassComputeFlags::INCLUDE_ALL_SHAPES);
        AZ_UNUSED(includeAllShapesInMassCalculation)
    }

    void RigidBody::BuildCompoundShape()
    {
        if (m_shapes.empty())
        {
            AZ_Error("JoltPhysics::RigidBody", false, "No shapes found to build compound shape on this rigid body: %s", GetName().c_str());
            return;
        }
        
        JPH::Ref<JPH::StaticCompoundShapeSettings> settings = new JPH::StaticCompoundShapeSettings;
        
        for (auto& shape : m_shapes)
        {
            auto* offsetShape = static_cast<JPH::RotatedTranslatedShape*>(shape->GetNativePointer());
            settings->AddShape(offsetShape->GetPosition(), offsetShape->GetRotation(), offsetShape->GetInnerShape());
        }
        
        auto result = settings->Create();
        
        if (result.HasError())
        {
            AZ_Error("JoltPhysics::RigidBody", false, "Failed to build compound shape on this rigid body: %s", GetName().c_str());
            return;
        }
        
        JPH::Ref<JPH::StaticCompoundShape> compoundShape = static_cast<JPH::StaticCompoundShape*>(result.Get().GetPtr());
        m_compoundShape = AZStd::make_shared<JoltPhysics::Shape>(compoundShape);
        
        {
            m_owningSystem->GetBodyInterface().SetShape(
                m_joltRigidBody->GetID(),
                compoundShape,
                true,
                JPH::EActivation::DontActivate
            );
        } 
        
        m_isReady = true;
    }

    AZ::u32 RigidBody::GetShapeCount() const
    {
        return static_cast<AZ::u32>(m_shapes.size());
    }

    AZStd::shared_ptr<Physics::Shape> RigidBody::GetShape(AZ::u32 index)
    {
        AZStd::shared_ptr<const Physics::Shape> constShape = AZStd::as_const(*this).GetShape(index);
        return AZStd::const_pointer_cast<Physics::Shape>(constShape);
    }

    AZStd::shared_ptr<const Physics::Shape> RigidBody::GetShape(AZ::u32 index) const
    {
        if (index >= m_shapes.size())
        {
            return nullptr;
        }
        return m_shapes[index];
    }

    AZ::Vector3 RigidBody::GetCenterOfMassWorld() const
    {
        if (m_joltRigidBody)
        {
            return JoltMathConvert(m_owningSystem->GetBodyInterface().GetCenterOfMassPosition(m_joltRigidBody->GetID()));
        }
        return AZ::Vector3::CreateZero();
    }

    AZ::Vector3 RigidBody::GetCenterOfMassLocal() const
    {
        if (m_joltRigidBody)
        {
            const JPH::Vec3 comWorld = m_owningSystem->GetBodyInterface().GetCenterOfMassPosition(m_joltRigidBody->GetID());
            const JPH::Vec3 worldTM = m_owningSystem->GetBodyInterface().GetWorldTransform(m_joltRigidBody->GetID()).GetTranslation();
            return JoltMathConvert(comWorld - worldTM);
        }
        return AZ::Vector3::CreateZero();
    }

    AZ::Matrix3x3 RigidBody::GetInertiaWorld() const
    {
        if (m_joltRigidBody)
        {
            return JoltMathConvert(m_owningSystem->GetBodyInterface().GetInverseInertia(m_joltRigidBody->GetID()).Inversed3x3());
        }
        return AZ::Matrix3x3::CreateZero();
    }

    AZ::Matrix3x3 RigidBody::GetInertiaLocal() const
    {
        if (m_joltRigidBody)
        {
            JPH::BodyLockRead lock(m_owningSystem->GetBodyLockInterface(), m_joltRigidBody->GetID());
            if (lock.Succeeded())
            {
                auto& body = lock.GetBody();
                return JoltMathConvert(body.GetMotionProperties()->GetLocalSpaceInverseInertia().Inversed3x3());
            }
        }

        return AZ::Matrix3x3::CreateZero();
    }

    AZ::Matrix3x3 RigidBody::GetInverseInertiaWorld() const
    {
        if (m_joltRigidBody)
        {
            return JoltMathConvert(m_owningSystem->GetBodyInterface().GetInverseInertia(m_joltRigidBody->GetID()));
        }
        return AZ::Matrix3x3::CreateZero();
    }

    AZ::Matrix3x3 RigidBody::GetInverseInertiaLocal() const
    {
        if (m_joltRigidBody)
        {
            JPH::BodyLockRead lock(m_owningSystem->GetBodyLockInterface(), m_joltRigidBody->GetID());
            if (lock.Succeeded())
            {
                auto& body = lock.GetBody();
                return JoltMathConvert(body.GetMotionProperties()->GetLocalSpaceInverseInertia());
            }
        }
        return AZ::Matrix3x3::CreateZero();
    }

    float RigidBody::GetMass() const
    {
        if (m_joltRigidBody)
        {
            return m_owningSystem->GetBodyInterface().GetShape(m_joltRigidBody->GetID())->GetMassProperties().mMass;
        }
        return 0.0f;
    }

    float RigidBody::GetInverseMass() const
    {
        // Kinematic bodies have infinite mass
        if (m_joltRigidBody && !m_joltRigidBody->IsKinematic())
        {
            JPH::BodyLockRead lock(m_owningSystem->GetBodyLockInterface(), m_joltRigidBody->GetID());
            if (lock.Succeeded())
            {
                auto& body = lock.GetBody();
                return body.GetMotionProperties()->GetInverseMass();
            }
        }
        return 0.0f;
    }
    // Mass is a calculated property and is preferentially set by material density and shape size
    void RigidBody::SetMass(float mass)
    {
        if (m_joltRigidBody && !m_joltRigidBody->IsKinematic())
        {
            JPH::BodyLockWrite lock(m_owningSystem->GetBodyLockInterface(), m_joltRigidBody->GetID());
            if (lock.Succeeded())
            {
                auto& body = lock.GetBody();
                return body.GetMotionProperties()->ScaleToMass(mass); // Scale's inertia vector based on old/new mass
            }
        }
    }

    void RigidBody::SetCenterOfMassOffset([[maybe_unused]] const AZ::Vector3& comOffset)
    {
    }

    AZ::Vector3 RigidBody::GetLinearVelocity() const
    {
        if (m_joltRigidBody)
        {
            return JoltMathConvert(m_owningSystem->GetBodyInterface().GetLinearVelocity(m_joltRigidBody->GetID()));
        }
        return AZ::Vector3::CreateZero();
    }

    void RigidBody::SetLinearVelocity(const AZ::Vector3& velocity)
    {
        if (m_joltRigidBody)
        {
            JPH::BodyLockWrite lock(m_owningSystem->GetBodyLockInterface(), m_joltRigidBody->GetID());
            if (lock.Succeeded())
            {
                JPH::Body& body = lock.GetBody();
                body.SetLinearVelocity(JoltMathConvert(velocity));
            }
        }
    }

    AZ::Vector3 RigidBody::GetAngularVelocity() const
    {
        if (m_joltRigidBody)
        {
            return JoltMathConvert(m_owningSystem->GetBodyInterface().GetAngularVelocity(m_joltRigidBody->GetID()));
        }
        return AZ::Vector3::CreateZero();
    }

    void RigidBody::SetAngularVelocity(const AZ::Vector3& angularVelocity)
    {
        if (m_joltRigidBody)
        {
            JPH::BodyLockWrite lock(m_owningSystem->GetBodyLockInterface(), m_joltRigidBody->GetID());
            if (lock.Succeeded())
            {
                auto& body = lock.GetBody();
                body.SetAngularVelocity(JoltMathConvert(angularVelocity));
            }
        }
    }

    AZ::Vector3 RigidBody::GetLinearVelocityAtWorldPoint([[maybe_unused]] const AZ::Vector3& worldPoint) const
    {
        AZ_Warning("RigidBody::GetLinearVelocityAtWorldPoint", false, "Not currently implemented")
        return AZ::Vector3::CreateZero();
    }

    void RigidBody::ApplyLinearImpulse(const AZ::Vector3& impulse)
    {
        if (m_joltRigidBody)
        {
            m_owningSystem->GetBodyInterface().AddImpulse(m_joltRigidBody->GetID(), JoltMathConvert(impulse));
        }
    }

    void RigidBody::ApplyLinearImpulseAtWorldPoint([[maybe_unused]] const AZ::Vector3& impulse, [[maybe_unused]] const AZ::Vector3& worldPoint)
    {
        AZ_Warning("RigidBody::ApplyLinearImpulseAtWorldPoint", false, "Not currently implemented")
    }

    void RigidBody::ApplyAngularImpulse(const AZ::Vector3& angularImpulse)
    {
        if (m_joltRigidBody)
        {
            m_owningSystem->GetBodyInterface().AddAngularImpulse(m_joltRigidBody->GetID(), JoltMathConvert(angularImpulse));
        }
    }

    bool RigidBody::IsKinematic() const
    {
        if (m_joltRigidBody)
        {
            return m_joltRigidBody->IsKinematic();
        }
        return false;
    }

    void RigidBody::SetKinematic(bool isKinematic)
    {
        if (isKinematic)
        {
            if (m_joltRigidBody && !m_joltRigidBody->IsKinematic())
            {
                m_owningSystem->GetBodyInterface().SetMotionType(m_joltRigidBody->GetID(), JPH::EMotionType::Kinematic, JPH::EActivation::Activate);
            }
        }
    }

    void RigidBody::SetKinematicTarget([[maybe_unused]] const AZ::Transform& targetPosition)
    {
        AZ_Warning("RigidBody::SetKinematicTarget", false, "Not currently implemented")
    }

    bool RigidBody::IsGravityEnabled() const
    {
        if (m_joltRigidBody)
        {
            JPH::BodyLockRead lock(m_owningSystem->GetBodyLockInterface(), m_joltRigidBody->GetID());
            if (lock.Succeeded())
            {
                auto& body = lock.GetBody();
                return body.GetMotionProperties()->GetGravityFactor() > 0.0f;
            }
        }
        return false;
    }

    void RigidBody::SetGravityEnabled([[maybe_unused]] bool enabled)
    {
        AZ_Warning("RigidBody::SetGravityEnabled", false, "Not currently implemented")
    }

    void RigidBody::SetSimulationEnabled([[maybe_unused]] bool enabled)
    {
        AZ_Warning("RigidBody::SetSimulationEnabled", false, "Not currently implemented")
    }

    void RigidBody::SetCCDEnabled([[maybe_unused]] bool enabled)
    {
        AZ_Warning("RigidBody::SetCCDEnabled", false, "Not currently implemented")
    }

    AZ::Transform RigidBody::GetTransform() const
    {
        if (m_joltRigidBody)
        {
            JPH::Vec3 outPosition = JPH::Vec3::sZero();
            JPH::Quat outRotation = JPH::Quat::sIdentity();
            m_owningSystem->GetBodyInterface().GetPositionAndRotation(m_joltRigidBody->GetID(), outPosition, outRotation);
            return JoltMathConvert(outPosition, outRotation);
        }
        return AZ::Transform::CreateIdentity();
    }

    void RigidBody::SetTransform(const AZ::Transform& transform)
    {
        if (m_joltRigidBody)
        {
            // This will only update transform if difference is larger than a very small number, to minimize necessary waking of BroadPhase Layer
            m_owningSystem->GetBodyInterface().SetPositionAndRotationWhenChanged(
                m_joltRigidBody->GetID(),
                JoltMathConvert(transform.GetTranslation()),
                JoltMathConvert(transform.GetRotation()),
                JPH::EActivation::Activate
                );
        }
    }

    AZ::Vector3 RigidBody::GetPosition() const
    {
        if (m_joltRigidBody)
        {
            return JoltMathConvert(m_owningSystem->GetBodyInterface().GetPosition(m_joltRigidBody->GetID()));
        }
        return AZ::Vector3::CreateZero();
    }

    AZ::Quaternion RigidBody::GetOrientation() const
    {
        if (m_joltRigidBody)
        {
            return JoltMathConvert(m_owningSystem->GetBodyInterface().GetRotation(m_joltRigidBody->GetID()));
        }
        return AZ::Quaternion::CreateZero();
    }

    AZ::Aabb RigidBody::GetAabb() const
    {
        if (m_joltRigidBody)
        {
            JPH::BodyLockRead lock(m_owningSystem->GetBodyLockInterface(), m_joltRigidBody->GetID());
            if (lock.Succeeded())
            {
                auto& body = lock.GetBody();
                return JoltMathConvert(body.GetWorldSpaceBounds());
            }
        }
        return AZ::Aabb::CreateNull();
    }

    AZ::EntityId RigidBody::GetEntityId() const
    {
        return m_bodyUserData.GetEntityId();
    }

    AzPhysics::SceneQueryHit RigidBody::RayCast(const AzPhysics::RayCastRequest& request)
    {
        AzPhysics::SceneQueryHit closestHit;
        
        if (!m_isReady)
        {
            AZ_Warning("RigidBody::RayCast", false, "Body is not finished activating yet")
            return closestHit;
        }
        
        float closestHitDist = AZStd::numeric_limits<float>::max();
        for (auto& shape : m_shapes) // TODO: maybe change to only call the compound shape
        {
            AzPhysics::SceneQueryHit hit = shape->RayCast(request, GetTransform());
            if (hit && hit.m_distance < closestHitDist)
            {
                closestHit = hit;
                closestHitDist = hit.m_distance;
            }
        }
        return closestHit;
    }

    AZ::Crc32 RigidBody::GetNativeType() const
    {
        return NativeTypeIdentifiers::RigidBody;
    }

    void* RigidBody::GetNativePointer() const
    {
        return m_joltRigidBody;
    }

    float RigidBody::GetLinearDamping() const
    {
        if (m_joltRigidBody)
        {
            JPH::BodyLockRead lock(m_owningSystem->GetBodyLockInterface(), m_joltRigidBody->GetID());
            if (lock.Succeeded())
            {
                auto& body = lock.GetBody();
                return body.GetMotionProperties()->GetLinearDamping();
            }
        }
        return 0.0f;
    }

    void RigidBody::SetLinearDamping(float damping)
    {
        if (damping < 0.0f)
        {
            AZ_Warning("Jolt Rigid Body", false, "Negative linear damping value (%6.4e). Name: %s", damping, GetName().c_str());
            return;
        }

        if (m_joltRigidBody)
        {
            JPH::BodyLockWrite lock(m_owningSystem->GetBodyLockInterface(), m_joltRigidBody->GetID());
            if (lock.Succeeded())
            {
                auto& body = lock.GetBody();
                return body.GetMotionProperties()->SetLinearDamping(damping);
            }
        }
    }

    float RigidBody::GetAngularDamping() const
    {
        if (m_joltRigidBody)
        {
            JPH::BodyLockRead lock(m_owningSystem->GetBodyLockInterface(), m_joltRigidBody->GetID());
            if (lock.Succeeded())
            {
                auto& body = lock.GetBody();
                return body.GetMotionProperties()->GetAngularDamping();
            }
        }
        return 0.0f;
    }

    void RigidBody::SetAngularDamping(float damping)
    {
        if (damping < 0.0f)
        {
            AZ_Warning("Jolt Rigid Body", false, "Negative angular damping value (%6.4e). Name: %s", damping, GetName().c_str());
            return;
        }

        if (m_joltRigidBody)
        {
            JPH::BodyLockWrite lock(m_owningSystem->GetBodyLockInterface(), m_joltRigidBody->GetID());
            if (lock.Succeeded())
            {
                auto& body = lock.GetBody();
                return body.GetMotionProperties()->SetLinearDamping(damping);
            }
        }
    }

    bool RigidBody::IsAwake() const
    {
        if (m_joltRigidBody)
        {
            return m_owningSystem->GetBodyInterface().IsActive(m_joltRigidBody->GetID());
        }
        return false;
    }

    void RigidBody::ForceAsleep()
    {
        if (m_joltRigidBody)
        {
            return m_owningSystem->GetBodyInterface().DeactivateBody(m_joltRigidBody->GetID());
        }
    }

    void RigidBody::ForceAwake()
    {
        if (m_joltRigidBody)
        {
            return m_owningSystem->GetBodyInterface().ActivateBody(m_joltRigidBody->GetID());
        }
    }

    // Sleep threshold is a global setting in Jolt
    float RigidBody::GetSleepThreshold() const
    {
        if (m_joltRigidBody)
        {
            return m_owningSystem->GetPhysicsSettings().mTimeBeforeSleep;
        }
        return 0.0f;
    }

    void RigidBody::SetSleepThreshold([[maybe_unused]] float threshold)
    {
        if (m_joltRigidBody)
        {
            AZ_Warning("RigidBody::SetSleepThreshold", false, "Not currently implemented")
        }
    }

    void RigidBody::SetName(const AZStd::string& entityName)
    {
        m_debugName = entityName;
    }

    const AZStd::string& RigidBody::GetName() const
    {
        return m_debugName;
    }
}
