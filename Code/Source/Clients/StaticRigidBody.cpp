#include "StaticRigidBody.h"

#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <AzCore/std/utility/as_const.h>
#include <AzFramework/Physics/Configuration/StaticRigidBodyConfiguration.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Utils.h>
#include <JoltPhysics/Utils.h>
#include <Clients/Shape.h>
#include <JoltPhysics/MathConversions.h>

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(JoltPhysics::StaticRigidBody, AZ::SystemAllocator);

    StaticRigidBody::StaticRigidBody(const AzPhysics::StaticRigidBodyConfiguration& configuration)
    {
        CreateJoltBody(configuration);
    }

    StaticRigidBody::~StaticRigidBody()
    {
        //clean up the attached shapes
        {
            PHYSX_SCENE_WRITE_LOCK(m_joltStaticBody->getScene());
            for (auto shape : m_shapes)
            {
                m_joltStaticBody->detachShape(*shape->GetPxShape());
                shape->DetachedFromActor();
            }
        }
        m_shapes.clear();

        // Invalidate user data so it sets m_pxStaticRigidBody->userData to nullptr.
        // It's appropriate to do this as m_pxStaticRigidBody is a shared pointer and
        // technically it could survive m_actorUserData life's span.
        m_actorUserData.Invalidate();
    }

    void StaticRigidBody::CreateJoltBody(const AzPhysics::StaticRigidBodyConfiguration& configuration)
    {
        if (m_joltStaticBody != nullptr)
        {
            AZ_Warning("Jolt Static Rigid Body", false, "Trying to create Jolt static rigid actor when it's already created");
            return;
        }

        if (m_joltStaticBody = PxActorFactories::CreatePxStaticRigidBody(configuration))
        {
            m_actorUserData = ActorData(m_joltStaticBody.get());
            m_actorUserData.SetRigidBodyStatic(this);
            m_actorUserData.SetEntityId(configuration.m_entityId);

            m_debugName = configuration.m_debugName;
            m_joltStaticBody->setName(m_debugName.c_str());
        }
    }

    void StaticRigidBody::AddShape(AZStd::shared_ptr<Physics::Shape> shape)
    {
        auto pxShape = AZStd::rtti_pointer_cast<JoltPhysics::Shape>(shape);
        if (pxShape && pxShape->GetPxShape())
        {
            {
                PHYSX_SCENE_WRITE_LOCK(m_joltStaticBody->getScene());
                m_joltStaticBody->attachShape(*pxShape->GetPxShape());
            }
            pxShape->AttachedToActor(m_joltStaticBody.get());
            m_shapes.push_back(pxShape);
        }
        else
        {
            AZ_Error("Jolt Rigid Body Static", false, "Trying to add an invalid shape.");
        }
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
            PHYSX_SCENE_READ_LOCK(m_joltStaticBody->getScene());
            return JoltMathConvert(m_joltStaticBody->getGlobalPose());
        }
        return AZ::Transform::CreateIdentity();
    }

    void StaticRigidBody::SetTransform(const AZ::Transform & transform)
    {
        if (m_joltStaticBody)
        {
            PHYSX_SCENE_WRITE_LOCK(m_joltStaticBody->getScene());
            m_joltStaticBody->setGlobalPose(JoltMathConvert(transform));
        }
    }

    AZ::Vector3 StaticRigidBody::GetPosition() const
    {
        if (m_joltStaticBody)
        {
            PHYSX_SCENE_READ_LOCK(m_joltStaticBody->getScene());
            return PxMathConvert(m_joltStaticBody->getGlobalPose().p);
        }
        return AZ::Vector3::CreateZero();
    }

    AZ::Quaternion StaticRigidBody::GetOrientation() const
    {
        if (m_joltStaticBody)
        {
            PHYSX_SCENE_READ_LOCK(m_joltStaticBody->getScene());
            return PxMathConvert(m_joltStaticBody->getGlobalPose().q);
        }
        return  AZ::Quaternion::CreateZero();
    }

    AZ::Aabb StaticRigidBody::GetAabb() const
    {
        if (m_joltStaticBody)
        {
            PHYSX_SCENE_READ_LOCK(m_joltStaticBody->getScene());
            return PxMathConvert(m_joltStaticBody->getWorldBounds(1.0f));
        }
        return AZ::Aabb::CreateNull();
    }

    AzPhysics::SceneQueryHit StaticRigidBody::RayCast(const AzPhysics::RayCastRequest& request)
    {
        return JoltPhysics::SceneQueryHelpers::ClosestRayHitAgainstShapes(request, m_shapes, GetTransform());
    }

    AZ::EntityId StaticRigidBody::GetEntityId() const
    {
        return m_actorUserData.GetEntityId();
    }

    AZ::Crc32 StaticRigidBody::GetNativeType() const
    {
        return JoltPhysics::NativeTypeIdentifiers::RigidBodyStatic;
    }

    void* StaticRigidBody::GetNativePointer() const
    {
        return m_joltStaticBody.get();
    }
}
