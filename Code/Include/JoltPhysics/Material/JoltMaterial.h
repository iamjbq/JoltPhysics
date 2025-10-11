
#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/smart_ptr/enable_shared_from_this.h>
#include <AzCore/Math/Color.h>

#include <AzFramework/Physics/Material/PhysicsMaterial.h>
#include <AzFramework/Physics/Material/PhysicsMaterialAsset.h>

namespace JPH
{
    class PhysicsMaterial;
}

namespace Physics
{
    class MaterialSlots;
};

namespace JoltPhysics
{
    class JoltPhysicsMaterial;
    
    //! Enumeration that determines how two materials properties are combined when
    //! processing collisions.
    enum class JoltCombineMode : AZ::u8
    {
        Average,
        Minimum,
        Maximum,
        Multiply,

        ENUM_COUNT
    };

    namespace MaterialConstants
    {
        inline constexpr AZStd::string_view MaterialAssetType = "Jolt";
        inline constexpr AZ::u32 MaterialAssetVersion = 1;

        inline constexpr AZStd::string_view DynamicFrictionName = "Friction";
        inline constexpr AZStd::string_view RestitutionName = "Restitution";
        inline constexpr AZStd::string_view DensityName = "Density";
        inline constexpr AZStd::string_view RestitutionCombineModeName = "RestitutionCombineMode";
        inline constexpr AZStd::string_view FrictionCombineModeName = "FrictionCombineMode";
        inline constexpr AZStd::string_view DebugColorName = "DebugColor";

        inline constexpr float MinDensityLimit = 0.01f; //!< Minimum possible value of density.
        inline constexpr float MaxDensityLimit = 100000.0f; //!< Maximum possible value of density.
    }
    
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
        AZ_CLASS_ALLOCATOR(Material, AZ::SystemAllocator)
        AZ_RTTI(JoltPhysics::Material, "{E7438567-6AE6-4142-BF25-0D9802777188}", Physics::Material)

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

        float GetFriction() const;
        void SetFriction(float friction);

        float GetRestitution() const;
        void SetRestitution(float restitution);

        float GetDensity() const;
        void SetDensity(float density);
        
        JoltCombineMode GetFrictionCombineMode() const;
        void SetFrictionCombineMode(JoltCombineMode mode);
        
        JoltCombineMode GetRestitutionCombineMode() const;
        void SetRestitutionCombineMode(JoltCombineMode mode);

        const AZ::Color& GetDebugColor() const;
        void SetDebugColor(const AZ::Color& debugColor);

        const JoltPhysicsMaterial* GetJoltMaterial() const; // TODO: implement own material class

    protected:
        // AssetBus overrides...
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;

        Material(
            const Physics::MaterialId& id,
            const AZ::Data::Asset<Physics::MaterialAsset>& materialAsset);
        
        using JoltMaterialUniquePtr = AZStd::unique_ptr<JoltPhysicsMaterial, AZStd::function<void(JoltPhysicsMaterial*)>>;

        JoltMaterialUniquePtr m_joltMaterial;
        float m_friction = 0.5;
        float m_restitution = 0.5f;
        float m_density = 1000.0f;
        AZ::Color m_debugColor = AZ::Colors::White;
    };
}
