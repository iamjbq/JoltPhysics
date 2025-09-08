
#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/smart_ptr/enable_shared_from_this.h>
#include <AzCore/Math/Color.h>

#include <AzFramework/Physics/Material/PhysicsMaterial.h>
#include <AzFramework/Physics/Material/PhysicsMaterialAsset.h>

namespace Physics
{
    class MaterialSlots;
};

namespace JoltPhysics
{
    //! Runtime Jolt material instance.
    //! It handles the reloading of its data if the material asset it
    //! was created from is modified.
    //! It also provides functions to create Jolt materials.
    class Material
        : public Physics::Material
        , public AZStd::enable_shared_from_this<Material>
        , protected AZ::Data::AssetBus::Handler
    {
    public:
        AZ_CLASS_ALLOCATOR(Material, AZ::SystemAllocator);
        AZ_RTTI(JoltPhysics::Material, "{E7438567-6AE6-4142-BF25-0D9802777188}", Physics::Material);

        //! Function to create a material instance from an asset.
        //! The material id will be constructed from the asset id.
        //! If the material id is found in the manager it returns the existing material instance.
        //! @param materialAsset Material asset to create the material instance from.
        //! @return Material instance created or found. It can return nullptr if the creation failed or if the asset passed is invalid.
        static AZStd::shared_ptr<Material> FindOrCreateMaterial(const AZ::Data::Asset<Physics::MaterialAsset>& materialAsset);

        //! Function to create material instances from material slots.
        //! The material ids will be constructed from the asset ids of the assets assigned to the slots.
        //! It will always return a valid list of materials, the slots with invalid or no assets will have
        //! the default material instance.
        //! @param materialSlots Material slots with the list of material assets to create the material instances from.
        //! @return List of material instances created. It will always return a valid list.
        static AZStd::vector<AZStd::shared_ptr<Material>> FindOrCreateMaterials(const Physics::MaterialSlots& materialSlots);

        //! Function to create a material instance from an asset.
        //! A random material will be used. This function is useful to create several instances from the same asset.
        //! @param materialAsset Material asset to create the material instance from.
        //! @return Material instance created. It can return nullptr if the creation failed or if the asset passed is invalid.
        static AZStd::shared_ptr<Material> CreateMaterialWithRandomId(const AZ::Data::Asset<Physics::MaterialAsset>& materialAsset);

        ~Material() override;

        // Physics::Material overrides ...
        Physics::MaterialPropertyValue GetProperty(AZStd::string_view propertyName) const override;
        void SetProperty(AZStd::string_view propertyName, Physics::MaterialPropertyValue value) override;

    protected:
        // AssetBus overrides...
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;

        Material(
            const Physics::MaterialId& id,
            const AZ::Data::Asset<Physics::MaterialAsset>& materialAsset);

        float m_restitution = 0.5f;
        float m_compliantContactModeDamping = 1.0f;
        float m_compliantContactModeStiffness = 1.0f;
        float m_density = 1000.0f;
        AZ::Color m_debugColor = AZ::Colors::White;
    };
}
