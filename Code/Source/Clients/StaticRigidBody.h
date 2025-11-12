#pragma once

#include <AzFramework/Physics/SimulatedBodies/StaticRigidBody.h>
#include <AzFramework/Physics/Shape.h>

#include <JoltPhysics/BodyData.h>

namespace JPH
{
    class Body;
    class PhysicsSystem;
}

namespace AzPhysics
{
    struct StaticRigidBodyConfiguration;
}

namespace JoltPhysics
{
    class Shape;

    class StaticRigidBody
        : public AzPhysics::StaticRigidBody
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_RTTI(JoltPhysics::StaticRigidBody, "{F07150C1-E805-469F-9383-5CD3D4BB373A}", AzPhysics::StaticRigidBody);

        StaticRigidBody() = default;
        StaticRigidBody(const AzPhysics::StaticRigidBodyConfiguration& configuration, JPH::PhysicsSystem* owningSystem);
        ~StaticRigidBody();

        // AzPhysics::StaticRigidBody
        void AddShape(AZStd::shared_ptr<Physics::Shape> shape) override;
        AZStd::shared_ptr<Physics::Shape> GetShape(AZ::u32 index) override;
        AZStd::shared_ptr<const Physics::Shape> GetShape(AZ::u32 index) const override;
        AZ::u32 GetShapeCount() const override;

        // AzPhysics::SimulatedBody
        AZ::EntityId GetEntityId() const override;

        AZ::Transform GetTransform() const override;
        void SetTransform(const AZ::Transform& transform) override;

        AZ::Vector3 GetPosition() const override;
        AZ::Quaternion GetOrientation() const override;

        AZ::Aabb GetAabb() const override;
        AzPhysics::SceneQueryHit RayCast(const AzPhysics::RayCastRequest& request) override;

        virtual AZ::Crc32 GetNativeType() const override;
        virtual void* GetNativePointer() const override;

    private:
        void CreateJoltBody(const AzPhysics::StaticRigidBodyConfiguration& configuration);

        JPH::PhysicsSystem* m_owningSystem = nullptr;
        JPH::Body* m_joltStaticBody = nullptr;
        AZStd::vector<AZStd::shared_ptr<JoltPhysics::Shape>> m_shapes;
        JoltPhysics::BodyData m_bodyUserData;
        AZStd::string m_debugName;
    };
} // namespace JoltPhysics
