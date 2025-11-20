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

#include <Utils.h>
#include <Clients/Shape.h>
#include <System/CollisionLayerFilters.h>
#include <JoltPhysics/Utils.h>
#include <JoltPhysics/MathConversions.h>
#include <JoltPhysics/NativeTypeIdentifiers.h>

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(JoltPhysics::StaticRigidBody, AZ::SystemAllocator);

    StaticRigidBody::StaticRigidBody(const AzPhysics::StaticRigidBodyConfiguration& configuration, JPH::PhysicsSystem* owningSystem)
        : m_owningSystem(owningSystem)
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

        auto newBody = JPH::BodyCreationSettings(
            emptyShape,
            JoltMathConvert(configuration.m_position),
            JoltMathConvert(configuration.m_orientation),
            JPH::EMotionType::Static,
            1 << 1 // Placeholder object layer until we set shape to get collider configuration
            );

        m_joltStaticBody = m_owningSystem->GetBodyInterface().CreateBody(newBody);
        m_owningSystem->GetBodyInterface().AddBody(m_joltStaticBody->GetID(), JPH::EActivation::DontActivate);

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
        if (joltShape && joltShape->GetNativePointer())
        {
            {
                m_owningSystem->GetBodyInterface().SetShape(
                    m_joltStaticBody->GetID(),
                    static_cast<const JPH::Shape*>(joltShape->GetNativePointer()),
                    true,
                    JPH::EActivation::DontActivate
                    );

                // This is a good place to set ObjectLayer since we can access collision layer/group, and we know body type (i.e. static)
                auto newBPLayer = JPH::BroadPhaseLayer(static_cast<AZ::u8>(JoltBroadPhaseLayer::Static));
                JPH::ObjectLayer newLayer = Utils::ConstructObjectLayer(shape->GetCollisionLayer(), shape->GetCollisionGroup(), newBPLayer);
                m_owningSystem->GetBodyInterface().SetObjectLayer(m_joltStaticBody->GetID(), newLayer);
            }
            joltShape->AttachedToActor(m_joltStaticBody);
            m_shapes.push_back(joltShape);
        }
        else
        {
            AZ_Error("Jolt Static Rigid Body", false, "Trying to add an invalid shape.");
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
}
