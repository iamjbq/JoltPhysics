#include "StaticRigidBody.h"

#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <AzCore/std/utility/as_const.h>
#include <AzFramework/Physics/Configuration/StaticRigidBodyConfiguration.h>

#include <Jolt/Jolt.h>
#include "Jolt/Physics/PhysicsSystem.h"
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Collision/Shape/EmptyShape.h"
#include "Jolt/Physics/Collision/Shape/StaticCompoundShape.h"

#include <Utils.h>
#include <Clients/Shape.h>
#include <System/CollisionLayerFilters.h>
#include <JoltPhysics/Utils.h>
#include <JoltPhysics/MathConversions.h>
#include <JoltPhysics/NativeTypeIdentifiers.h>

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(JoltPhysics::StaticRigidBody, AZ::SystemAllocator);

    StaticRigidBody::StaticRigidBody(const AzPhysics::StaticRigidBodyConfiguration& configuration, JPH::PhysicsSystem& owningSystem)
        : m_owningSystem(&owningSystem)
    {
        CreateJoltBody(configuration);
    }

    StaticRigidBody::~StaticRigidBody()
    {
        //clean up the attached shapes
        {
            for (auto shape : m_shapes)
            {
                // We may need to mark on m_joltStaticBody that it is to be deleted
                // TODO: Set body shape to EmptyShape again?
                shape->DetachedFromActor();
            }
        }
        m_shapes.clear();

        // Invalidate user data so it sets m_joltStaticBody->userData to nullptr.
        // It's appropriate to do this as m_joltStaticBody is a shared pointer, and
        // technically it could survive m_actorUserData life's span.
        m_bodyUserData.Invalidate();
    }

    void StaticRigidBody::CreateJoltBody(const AzPhysics::StaticRigidBodyConfiguration& configuration)
    {
        if (m_joltStaticBody != nullptr)
        {
            AZ_Warning("Jolt Static Rigid Body", false, "Trying to create Jolt static rigid actor when it's already created");
            return;
        }
        // We can't crate a Body without a shape. This will be replaced in AddShape()
        JPH::EmptyShapeSettings emptySettings;
        JPH::Shape* emptyShape = emptySettings.Create().Get();

        JPH::BodyCreationSettings newBody;
        newBody.SetShape(emptyShape);
        newBody.mPosition = JoltMathConvert(configuration.m_position);
        newBody.mRotation = JoltMathConvert(configuration.m_orientation);
        newBody.mMotionType = JPH::EMotionType::Static,
        newBody.mObjectLayer = 1 << 1; // Placeholder object layer until ApplyJoltConfiguration()

        m_joltStaticBody = m_owningSystem->GetBodyInterface().CreateBody(newBody);
        // m_owningSystem->GetBodyInterface().AddBody(m_joltStaticBody->GetID(), JPH::EActivation::DontActivate);

        if (m_joltStaticBody == nullptr)
        {
            AZ_Warning("StaticRigidBody::CreateJoltBody", false, "Jolt Body pointer was null")
        }

        m_bodyUserData = BodyData(m_joltStaticBody);
        m_bodyUserData.SetRigidBodyStatic(this);

        m_bodyUserData.SetEntityId(configuration.m_entityId);

        m_debugName = configuration.m_debugName;
    }

    // This gets called in JoltScene directly after creating a StaticRigidBody
    void StaticRigidBody::AddShape(AZStd::shared_ptr<Physics::Shape> shape)
    {
        if (!m_joltStaticBody || !shape)
        {
            return;
        }

        auto joltShape = AZStd::rtti_pointer_cast<JoltPhysics::Shape>(shape);
        
        if (!joltShape)
        {
            AZ_Error("Jolt Static Rigid Body", false, "Trying to add a shape of unknown type. Name: %s", GetName().c_str());
            return;
        }

        if (!joltShape->GetNativePointer())
        {
            AZ_Error("Jolt Static Rigid Body", false, "Trying to add a shape with no valid JPH::Shape. Name: %s", GetName().c_str());
            return;
        }
            
        joltShape->SetInternalPhysicsSystem(m_owningSystem);
        joltShape->AttachedToActor(m_joltStaticBody);
        m_shapes.push_back(joltShape);
    }

    AZStd::shared_ptr<Physics::Shape> StaticRigidBody::GetShape(AZ::u32 index)
    {
        AZStd::shared_ptr<const Physics::Shape> constShape = AZStd::as_const(*this).GetShape(index);
        return AZStd::const_pointer_cast<Physics::Shape>(constShape);
    }

    AZStd::shared_ptr<const Physics::Shape> StaticRigidBody::GetShape(AZ::u32 index) const
    {
        if (index >= m_shapes.size())
        {
            return nullptr;
        }
        return m_shapes[index];
    }

    AZ::u32 StaticRigidBody::GetShapeCount() const
    {
        return static_cast<AZ::u32>(m_shapes.size());
    }

    AZ::Transform StaticRigidBody::GetTransform() const
    {
        if (m_joltStaticBody)
        {
            JPH::Mat44 transform = m_joltStaticBody->GetWorldTransform();
            return JoltMathConvert(transform.GetTranslation(), transform.GetQuaternion());
        }
        return AZ::Transform::CreateIdentity();
    }

    void StaticRigidBody::SetTransform(const AZ::Transform & transform)
    {
        if (m_joltStaticBody)
        {
            m_owningSystem->GetBodyInterface().SetPositionAndRotation(
                m_joltStaticBody->GetID(),
                JoltMathConvert(transform.GetTranslation()),
                JoltMathConvert(transform.GetRotation()),
                JPH::EActivation::Activate
                );
        }
    }

    AZ::Vector3 StaticRigidBody::GetPosition() const
    {
        if (m_joltStaticBody)
        {
            return JoltMathConvert(m_owningSystem->GetBodyInterface().GetPosition(m_joltStaticBody->GetID()));
        }
        return AZ::Vector3::CreateZero();
    }

    AZ::Quaternion StaticRigidBody::GetOrientation() const
    {
        if (m_joltStaticBody)
        {
            return JoltMathConvert(m_owningSystem->GetBodyInterface().GetRotation(m_joltStaticBody->GetID()));
        }
        return  AZ::Quaternion::CreateZero();
    }

    AZ::Aabb StaticRigidBody::GetAabb() const
    {
        if (m_joltStaticBody)
        {
            JPH::BodyLockRead lock(m_owningSystem->GetBodyLockInterface(), m_joltStaticBody->GetID());
            if (lock.Succeeded())
            {
                auto& body = lock.GetBody();
                return JoltMathConvert(body.GetWorldSpaceBounds());
            }
        }
        return AZ::Aabb::CreateNull();
    }

    AzPhysics::SceneQueryHit StaticRigidBody::RayCast(const AzPhysics::RayCastRequest& request)
    {
        AzPhysics::SceneQueryHit closestHit;
        float closestHitDist = AZStd::numeric_limits<float>::max();
        for (auto& shape : m_shapes)
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

    AZ::EntityId StaticRigidBody::GetEntityId() const
    {
        return m_bodyUserData.GetEntityId();
    }

    AZ::Crc32 StaticRigidBody::GetNativeType() const
    {
        return JoltPhysics::NativeTypeIdentifiers::RigidBodyStatic;
    }

    void* StaticRigidBody::GetNativePointer() const
    {
        return m_joltStaticBody;
    }

    void StaticRigidBody::BuildCompoundShape()
    {
        if (m_shapes.empty())
        {
            AZ_Error("JoltPhysics::StaticRigidBody", false, "No shapes found to build compound shape on this rigid body: %s", GetName().c_str());
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
            AZ_Error("JoltPhysics::StaticRigidBody", false, "Failed to build compound shape on this rigid body: %s", GetName().c_str());
            return;
        }
        
        JPH::Ref<JPH::StaticCompoundShape> compoundShape = static_cast<JPH::StaticCompoundShape*>(result.Get().GetPtr());
        m_compoundShape = AZStd::make_shared<JoltPhysics::Shape>(compoundShape);
        
        {
            m_owningSystem->GetBodyInterface().SetShape(
                m_joltStaticBody->GetID(),
                compoundShape,
                true,
                JPH::EActivation::DontActivate
                );
        }
    }
    
    void StaticRigidBody::SetName(const AZStd::string& entityName)
    {
        m_debugName = entityName;
    }

    const AZStd::string& StaticRigidBody::GetName() const
    {
        return m_debugName;
    }
}
