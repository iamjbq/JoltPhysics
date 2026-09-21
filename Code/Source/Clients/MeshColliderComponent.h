
#pragma once

#include <JoltPhysics/MeshColliderComponentBus.h>
#include <Clients/BaseColliderComponent.h>

namespace JoltPhysics
{
    class MeshColliderComponent
        : public BaseColliderComponent
        , public MeshColliderComponentRequestsBus::Handler
        , public AZ::Data::AssetBus::MultiHandler
    {
    public:
        AZ_COMPONENT(MeshColliderComponent, "{46BA644B-0204-45FD-BF99-5F2F15831FD2}", BaseColliderComponent);
        static void Reflect(AZ::ReflectContext* context);

        MeshColliderComponent() = default;
        ~MeshColliderComponent() override = default;

        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        // AZ::Data::AssetBus::Handler
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;

        // MeshColliderComponentRequestsBus
        AZ::Data::Asset<Pipeline::MeshAsset> GetMeshAsset() const override;
        void SetMeshAsset(const AZ::Data::AssetId& id) override;

        // BaseColliderComponent
        void UpdateScaleForShapeConfigs() override;

    protected:
        void UpdateMeshAsset();

        Physics::ColliderConfiguration* m_colliderConfiguration = nullptr;
        Physics::PhysicsAssetShapeConfiguration* m_shapeConfiguration = nullptr;
    };
} // namespace JoltPhysics
