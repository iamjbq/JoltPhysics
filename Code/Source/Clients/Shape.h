
#pragma once

#include <AzFramework/Physics/Shape.h>
#include <AzFramework/Physics/ShapeConfiguration.h>
#include <AzCore/std/smart_ptr/enable_shared_from_this.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>

#include <AzFramework/Physics/Collision/CollisionGroups.h>
#include <AzFramework/Physics/Collision/CollisionLayers.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>


namespace JPH
{
    class ShapeSettings;
}

namespace Physics
{
    class Material;
}

namespace JoltPhysics
{
    class JoltScene;

    class Shape
        : public Physics::Shape
        , public AZStd::enable_shared_from_this<Shape>
    {
    public:
        AZ_CLASS_ALLOCATOR(Shape, AZ::SystemAllocator, 0);
        AZ_RTTI(Shape, "{92AC6CAF-4C88-4BFF-B2EF-D2C1F740918D}", Physics::Shape)

        Shape(const Physics::ColliderConfiguration& colliderConfiguration, const Physics::ShapeConfiguration& configuration);
        Shape(JPH::ShapeSettings* nativeShape);
        virtual ~Shape();

        Shape(Shape&& shape);
        Shape& operator=(Shape&& shape);
        Shape(const Shape& shape) = delete;
        Shape& operator=(const Shape& shape) = delete;

        // Physics::Shape overrides...
        void SetMaterial(const AZStd::shared_ptr<Physics::Material>& material) override;
        AZStd::shared_ptr<Physics::Material> GetMaterial() const override;
        Physics::MaterialId GetMaterialId() const override;
        void SetCollisionLayer(const AzPhysics::CollisionLayer& layer) override;
        AzPhysics::CollisionLayer GetCollisionLayer() const override;
        void SetCollisionGroup(const AzPhysics::CollisionGroup& group) override;
        AzPhysics::CollisionGroup GetCollisionGroup() const override;
        void SetName(const char* name) override;
        void SetLocalPose(const AZ::Vector3& offset, const AZ::Quaternion& rotation) override;
        AZStd::pair<AZ::Vector3, AZ::Quaternion> GetLocalPose() const override;
        float GetRestOffset() const override;
        float GetContactOffset() const override;
        void SetRestOffset(float restOffset) override;
        void SetContactOffset(float contactOffset) override;
        void* GetNativePointer() override;
        const void* GetNativePointer() const override;
        AZ::Crc32 GetTag() const override;
        void AttachedToActor(void* actor) override;
        void DetachedFromActor() override;
        AzPhysics::SceneQueryHit RayCast(const AzPhysics::RayCastRequest& worldSpaceRequest, const AZ::Transform& worldTransform) override;
        AzPhysics::SceneQueryHit RayCastLocal(const AzPhysics::RayCastRequest& localSpaceRequest) override;
        AZ::Aabb GetAabb(const AZ::Transform& worldTransform) const override;
        AZ::Aabb GetAabbLocal() const override;
        // AZStd::shared_ptr<Physics::ShapeConfiguration> GetShapeConfiguration() const override;
        void GetGeometry(AZStd::vector<AZ::Vector3>& vertices, AZStd::vector<AZ::u32>& indices,
        const AZ::Aabb* optionalBounds = nullptr) const override;

        JPH::Shape* GetJoltShape();

    private:
        JoltPhysics::JoltScene* GetScene();

        AzPhysics::CollisionLayer m_collisionLayer;
        AzPhysics::CollisionGroup m_collisionGroup;
        AZStd::shared_ptr<Physics::ShapeConfiguration> m_shapeConfiguration;
        AZ::Crc32 m_tag;

        Shape() = default;
    };
} // JoltPhysics
