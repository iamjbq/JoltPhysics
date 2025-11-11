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
#include "Jolt/Physics/Collision/Shape/SphereShape.h"

#include <Utils.h>
#include <Clients/Shape.h>
#include <System/CollisionLayerFilters.h>
#include <JoltPhysics/Utils.h>
#include <JoltPhysics/MathConversions.h>
#include <JoltPhysics/NativeTypeIdentifiers.h>

#include "Jolt/Physics/Body/BodyLock.h"

namespace JoltPhysics
{
    void RigidBodyConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<JoltPhysics::RigidBodyConfiguration>()
                ->Version(1)
                // ->Field("SolverPositionIterations", &JoltPhysics::RigidBodyConfiguration::m_solverPositionIterations)
                // ->Field("SolverVelocityIterations", &JoltPhysics::RigidBodyConfiguration::m_solverVelocityIterations)
                ;

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<JoltPhysics::RigidBodyConfiguration>("JoltPhysics-specific Rigid Body Configuration",
                    "Additional Rigid Body settings specific to JoltPhysics.")
                    // ->DataElement(
                    //     AZ::Edit::UIHandlers::Default,
                    //     &JoltPhysics::RigidBodyConfiguration::m_solverPositionIterations,
                    //     "Solver Position Iterations",
                    //     "Higher values can improve stability at the cost of performance.")
                    // ->Attribute(AZ::Edit::Attributes::Min, 1)
                    // ->Attribute(AZ::Edit::Attributes::Max, 255)
                    // ->DataElement(
                    //     AZ::Edit::UIHandlers::Default,
                    //     &JoltPhysics::RigidBodyConfiguration::m_solverVelocityIterations,
                    //     "Solver Velocity Iterations",
                    //     "Higher values can improve stability at the cost of performance.")
                    // ->Attribute(AZ::Edit::Attributes::Min, 1)
                    // ->Attribute(AZ::Edit::Attributes::Max, 255)
                    ;
            }
        }
    }

    RigidBody::RigidBody(const AzPhysics::RigidBodyConfiguration& configuration, JPH::PhysicsSystem& owningSystem)
        : m_owningSystem(owningSystem)
    {
        CreateJoltBody(configuration);
    }

    RigidBody::~RigidBody()
    {
        //clean up the attached shapes
        {
            for (auto shape : m_shapes)
            {
                // Jolt Body cannot change or remove shape after creation
                // We may need to mark on m_joltStaticBody that it is to be deleted

                shape->DetachedFromActor();
            }
        }
        m_shapes.clear();

        // Invalidate user data so it sets m_joltStaticBody->userData to nullptr.
        // It's appropriate to do this as m_joltStaticBody is a shared pointer, and
        // technically it could survive m_actorUserData life's span.
        m_bodyUserData.Invalidate();
    }

    void RigidBody::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<RigidBody>()
                ->Version(1)
            ;
        }
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

    float RigidBody::GetMass() const
    {
        if (m_joltRigidBody)
        {
            return m_owningSystem.GetBodyInterface().GetShape(m_joltRigidBody->GetID())->GetMassProperties().mMass;
        }
        return 0.0f;
    }

    float RigidBody::GetInverseMass() const
    {
        // Kinematic bodies have infinite mass
        if (m_joltRigidBody && !m_joltRigidBody->IsKinematic())
        {
            JPH::BodyLockRead lock(m_owningSystem.GetBodyLockInterface(), m_joltRigidBody->GetID());
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
            JPH::BodyLockWrite lock(m_owningSystem.GetBodyLockInterface(), m_joltRigidBody->GetID());
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
            return JoltMathConvert(m_owningSystem.GetBodyInterface().GetLinearVelocity(m_joltRigidBody->GetID()));
        }
        return AZ::Vector3::CreateZero();
    }

    void RigidBody::SetLinearVelocity(const AZ::Vector3& velocity)
    {
        if (m_joltRigidBody)
        {
            JPH::BodyLockWrite lock(m_owningSystem.GetBodyLockInterface(), m_joltRigidBody->GetID());
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
            return JoltMathConvert(m_owningSystem.GetBodyInterface().GetAngularVelocity(m_joltRigidBody->GetID()));
        }
        return AZ::Vector3::CreateZero();
    }

    void RigidBody::SetAngularVelocity(const AZ::Vector3& angularVelocity)
    {
        if (m_joltRigidBody)
        {
            JPH::BodyLockWrite lock(m_owningSystem.GetBodyLockInterface(), m_joltRigidBody->GetID());
            if (lock.Succeeded())
            {
                JPH::Body& body = lock.GetBody();
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
            m_owningSystem.GetBodyInterface().AddImpulse(m_joltRigidBody->GetID(), JoltMathConvert(impulse));
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
            m_owningSystem.GetBodyInterface().AddAngularImpulse(m_joltRigidBody->GetID(), JoltMathConvert(angularImpulse));
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
        if (m_joltRigidBody && !m_joltRigidBody->IsKinematic())
        {
            m_owningSystem.GetBodyInterface().SetMotionType(m_joltRigidBody->GetID(), JPH::EMotionType::Kinematic, JPH::EActivation::Activate);
        }
    }

    void RigidBody::SetKinematicTarget([[maybe_unused]] const AZ::Transform& targetPosition)
    {
        AZ_Warning("RigidBody::SetKinematicTarget", false, "Not currently implemented")
    }

    AZ::Transform RigidBody::GetTransform() const
    {
        if (m_joltRigidBody)
        {
            JPH::Vec3 outPosition;
            JPH::Quat outRotation;
            m_owningSystem.GetBodyInterface().GetPositionAndRotation(m_joltRigidBody->GetID(), outPosition, outRotation);
            return JoltMathConvert(outPosition, outRotation);
        }
        return AZ::Transform::CreateIdentity();
    }

    void RigidBody::SetTransform(const AZ::Transform& transform)
    {
        if (m_joltRigidBody)
        {
            // This will only update transform if difference is larger than a very small number, to minimize necessary waking of BroadPhase Layer
            m_owningSystem.GetBodyInterface().SetPositionAndRotationWhenChanged(
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
            return JoltMathConvert(m_owningSystem.GetBodyInterface().GetPosition(m_joltRigidBody->GetID()));
        }
        return AZ::Vector3::CreateZero();
    }

    AZ::Quaternion RigidBody::GetOrientation() const
    {
        if (m_joltRigidBody)
        {
            return JoltMathConvert(m_owningSystem.GetBodyInterface().GetRotation(m_joltRigidBody->GetID()));
        }
        return AZ::Quaternion::CreateZero();
    }

    AZ::Aabb RigidBody::GetAabb() const
    {
        if (m_joltRigidBody)
        {
            JPH::BodyLockRead lock(m_owningSystem.GetBodyLockInterface(), m_joltRigidBody->GetID());
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
            JPH::BodyLockRead lock(m_owningSystem.GetBodyLockInterface(), m_joltRigidBody->GetID());
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
            JPH::BodyLockWrite lock(m_owningSystem.GetBodyLockInterface(), m_joltRigidBody->GetID());
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
            JPH::BodyLockRead lock(m_owningSystem.GetBodyLockInterface(), m_joltRigidBody->GetID());
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
            JPH::BodyLockWrite lock(m_owningSystem.GetBodyLockInterface(), m_joltRigidBody->GetID());
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
            return m_owningSystem.GetBodyInterface().IsActive(m_joltRigidBody->GetID());
        }
        return false;
    }

    void RigidBody::ForceAsleep()
    {
        if (m_joltRigidBody)
        {
            return m_owningSystem.GetBodyInterface().DeactivateBody(m_joltRigidBody->GetID());
        }
    }

    void RigidBody::ForceAwake()
    {
        if (m_joltRigidBody)
        {
            return m_owningSystem.GetBodyInterface().ActivateBody(m_joltRigidBody->GetID());
        }
    }

    // Sleep threshold is a global setting in Jolt
    float RigidBody::GetSleepThreshold() const
    {
        if (m_joltRigidBody)
        {
            return m_owningSystem.GetPhysicsSettings().mTimeBeforeSleep;
        }
        return 0.0f;
    }

    void RigidBody::SetSleepThreshold(float threshold)
    {
        if (m_joltRigidBody)
        {
            AZ_Warning("RigidBody::SetSleepThreshold", false, "Not currently implemented")
        }
    }

    void RigidBody::SetName(const AZStd::string& entityName)
    {
        m_debugName = entityName;
        // TODO: Check if Jolt has debug naming
    }

    const AZStd::string& RigidBody::GetName() const
    {
        return m_debugName;
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
            // Actually not sure if this is true for Jolt, but leaving it here until otherwise
            AZ_Error("Jolt", false, "Cannot use mesh geometry on a dynamic object: %s", GetName().c_str());
            return;
        }

        {
            m_owningSystem.GetBodyInterface().SetShape(
                m_joltRigidBody->GetID(),
                static_cast<const JPH::Shape*>(joltShape->GetNativePointer()),
                true,
                JPH::EActivation::DontActivate // TODO: Probably a check whether this should be activated
                );

            // This is a good place to set ObjectLayer since we can access collision layer/group, and we know body type (i.e. dynamic)
            JPH::ObjectLayer newLayer = GetNewObjectLayer(joltShape);
            m_owningSystem.GetBodyInterface().SetObjectLayer(m_joltRigidBody->GetID(), newLayer);
        }
        joltShape->AttachedToActor(m_joltRigidBody);
        m_shapes.push_back(joltShape);
    }

    void RigidBody::RemoveShape([[maybe_unused]] AZStd::shared_ptr<Physics::Shape> shape)
    {
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

    void RigidBody::UpdateMassProperties(
        AzPhysics::MassComputeFlags flags,
        const AZ::Vector3& centerOfMassOffsetOverride,
        const AZ::Matrix3x3& inertiaTensorOverride,
        const float massOverride)
    {
        const bool computeCenterOfMass = AzPhysics::MassComputeFlags::COMPUTE_COM == (flags & AzPhysics::MassComputeFlags::COMPUTE_COM);
        const bool computeInertiaTensor = AzPhysics::MassComputeFlags::COMPUTE_INERTIA == (flags & AzPhysics::MassComputeFlags::COMPUTE_INERTIA);
        const bool computeMass = AzPhysics::MassComputeFlags::COMPUTE_MASS == (flags & AzPhysics::MassComputeFlags::COMPUTE_MASS);
        const bool needsCompute = computeCenterOfMass || computeInertiaTensor || computeMass;
        const bool includeAllShapesInMassCalculation = AzPhysics::MassComputeFlags::INCLUDE_ALL_SHAPES == (flags & AzPhysics::MassComputeFlags::INCLUDE_ALL_SHAPES);
    }

    void RigidBody::CreateJoltBody(const AzPhysics::RigidBodyConfiguration& configuration)
    {
        if (m_joltRigidBody != nullptr)
        {
            AZ_Warning("Jolt Rigid Body", false, "Trying to create Jolt rigid actor when it's already created");
            return;
        }
        // We can't crate a Body without a shape. Create a tiny sphere as placeholder
        JPH::SphereShapeSettings placeholderSettings(0.01f);  // 1cm radius
        JPH::Shape::ShapeResult result = placeholderSettings.Create();

        JPH::EMotionType motionType;
        if (configuration.m_kinematic)
        {
            motionType = JPH::EMotionType::Kinematic;
        }
        else
        {
            motionType = JPH::EMotionType::Dynamic;
        }

        auto newBody = JPH::BodyCreationSettings(
            result.Get(),
            JoltMathConvert(configuration.m_position),
            JoltMathConvert(configuration.m_orientation),
            motionType,
            0 // Placeholder object layer until we set shape to get collider configuration
            );

        m_joltRigidBody = m_owningSystem.GetBodyInterface().CreateBody(newBody);

        if (m_joltRigidBody)
        {
            m_bodyUserData = BodyData(m_joltRigidBody); // TODO: find out when user data is set on Jolt Body
            m_bodyUserData.SetRigidBody(this);

            m_bodyUserData.SetEntityId(configuration.m_entityId);

            m_debugName = configuration.m_debugName;
        }
    }

    JPH::ObjectLayer RigidBody::GetNewObjectLayer(const AZStd::shared_ptr<Shape>& shape)
    {
        auto newBPLayer = JPH::BroadPhaseLayer(static_cast<AZ::u8>(JoltBroadPhaseLayer::Dynamic));
        return Utils::ConstructObjectLayer(shape->GetCollisionLayer(), shape->GetCollisionGroup(), newBPLayer);
    }
}
