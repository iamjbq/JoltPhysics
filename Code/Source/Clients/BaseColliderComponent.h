#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TransformBus.h>

#include <AzFramework/Physics/CollisionBus.h>

#include <JoltPhysics/ColliderComponentBus.h>
#include <JoltPhysics/ColliderShapeBus.h>

#include <Clients/Shape.h>

namespace JoltPhysics
{
    class StaticRigidBody;

    /// Base class for all runtime collider components.
    class BaseColliderComponent
        : public AZ::Component
        , public ColliderComponentRequestBus::Handler
        , private AZ::TransformNotificationBus::Handler
        , protected JoltPhysics::ColliderShapeRequestBus::Handler
        , protected Physics::CollisionFilteringRequestBus::Handler
    {
    public:
        AZ_COMPONENT(BaseColliderComponent, "{DE3555C7-B3B3-4FE9-AAB6-EC21619DADC1}");
        static void Reflect(AZ::ReflectContext* context);

        BaseColliderComponent() = default;

        void SetShapeConfigurationList(const AzPhysics::ShapeColliderPairList& shapeConfigList);

        // ColliderComponentRequestBus
        AzPhysics::ShapeColliderPairList GetShapeConfigurations() override;
        AZStd::vector<AZStd::shared_ptr<Physics::Shape>> GetShapes() override;

        // TransformNotificationsBus
        void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;

        // JoltPhysics::ColliderShapeBus
        AZ::Aabb GetColliderShapeAabb() override;
        bool IsTrigger() override;

        // CollisionFilteringRequestBus
        void SetCollisionLayer(const AZStd::string& layerName, AZ::Crc32 filterTag) override;
        AZStd::string GetCollisionLayerName() override;
        void SetCollisionGroup(const AZStd::string& groupName, AZ::Crc32 filterTag) override;
        AZStd::string GetCollisionGroupName() override;
        void ToggleCollisionLayer(const AZStd::string& layerName, AZ::Crc32 filterTag, bool enabled) override;

    protected:
        /// Class for collider's shape parameters that are cached.
        /// Caching can also be done per WorldBody.
        /// That can be and should be achieved after m_staticRigidBody is separated from the collider component.
        class ShapeInfoCache
        {
        public:
            AZ::Aabb GetAabb(const AZStd::vector<AZStd::shared_ptr<Physics::Shape>>& shapes);

            void InvalidateCache();

            const AZ::Transform& GetWorldTransform();

            void SetWorldTransform(const AZ::Transform& worldTransform);

        private:
            void UpdateCache(const AZStd::vector<AZStd::shared_ptr<Physics::Shape>>& shapes);

            AZ::Aabb m_aabb = AZ::Aabb::CreateNull();
            AZ::Transform m_worldTransform = AZ::Transform::CreateIdentity();
            bool m_cacheOutdated = true;
        };

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
        {
            provided.push_back(AZ_CRC_CE("PhysicsColliderService"));
            provided.push_back(AZ_CRC_CE("PhysicsTriggerService"));
        }

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
        {
            dependent.push_back(AZ_CRC_CE("NonUniformScaleService"));
        }

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
            required.push_back(AZ_CRC_CE("TransformService"));
        }

        static void GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
        {

        }

        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        /// Updates the scale of shape configurations to reflect the scale from the transform component.
        /// Specific collider components should override this function.
        virtual void UpdateScaleForShapeConfigs();

        ShapeInfoCache m_shapeInfoCache;
        AzPhysics::ShapeColliderPairList m_shapeConfigList;
    private:
        bool InitShapes();
        bool IsMeshCollider() const;
        bool InitMeshCollider();

        AZStd::vector<AZStd::shared_ptr<Physics::Shape>> m_shapes;
    };
}
