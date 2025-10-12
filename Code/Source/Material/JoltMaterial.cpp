#include <AzCore/Interface/Interface.h>

#include <AzFramework/Physics/Material/PhysicsMaterialSlots.h>
#include <AzFramework/Physics/Material/PhysicsMaterialManager.h>

#include <Jolt/Jolt.h>

#include <JoltPhysics/Material/JoltMaterial.h>
#include <JoltPhysics/Material/JoltMaterialConfiguration.h>
#include <Material/JoltPhysicsMaterial.h>


namespace JoltPhysics
{
    static CombineMode FromJoltCombineMode(JoltPhysics::JoltCombineMode::Enum joltMode)
    {
        switch (joltMode)
        {
        case JoltCombineMode::Average:
            return CombineMode::Average;
        case JoltCombineMode::Multiply:
            return CombineMode::Multiply;
        case JoltCombineMode::Maximum:
            return CombineMode::Maximum;
        case JoltCombineMode::Minimum:
            return CombineMode::Minimum;
        default:
            return CombineMode::Average;
        }
    }

    static JoltPhysics::JoltCombineMode::Enum ToJoltCombineMode(CombineMode mode)
    {
        switch (mode)
        {
        case CombineMode::Average:
            return JoltCombineMode::Average;
        case CombineMode::Multiply:
            return JoltCombineMode::Multiply;
        case CombineMode::Maximum:
            return JoltCombineMode::Maximum;
        case CombineMode::Minimum:
            return JoltCombineMode::Minimum;
        default:
            return JoltCombineMode::Maximum;
        }
    }

    AZStd::shared_ptr<Material> Material::FindOrCreateMaterial(const AZ::Data::Asset<Physics::MaterialAsset>& materialAsset)
    {
        return AZStd::rtti_pointer_cast<Material>(
            AZ::Interface<Physics::MaterialManager>::Get()->FindOrCreateMaterial(
                Physics::MaterialId::CreateFromAssetId(materialAsset.GetId()),
                materialAsset));
    }

    AZStd::vector<AZStd::shared_ptr<Material>> Material::FindOrCreateMaterials(const Physics::MaterialSlots& materialSlots)
    {
        AZStd::shared_ptr<Material> defaultMaterial =
            AZStd::rtti_pointer_cast<Material>(
                AZ::Interface<Physics::MaterialManager>::Get()->GetDefaultMaterial());

        const size_t slotsCount = materialSlots.GetSlotsCount();

        AZStd::vector<AZStd::shared_ptr<Material>> materials;
        materials.reserve(slotsCount);

        for (size_t slotIndex = 0; slotIndex < slotsCount; ++slotIndex)
        {
            if (const auto materialAsset = materialSlots.GetMaterialAsset(slotIndex);
                materialAsset.GetId().IsValid())
            {
                auto material = Material::FindOrCreateMaterial(materialAsset);
                if (material)
                {
                    materials.push_back(material);
                }
                else
                {
                    materials.push_back(defaultMaterial);
                }
            }
            else
            {
                materials.push_back(defaultMaterial);
            }
        }

        return materials;
    }

    AZStd::shared_ptr<Material> Material::CreateMaterialWithRandomId(const AZ::Data::Asset<Physics::MaterialAsset>& materialAsset)
    {
        return AZStd::rtti_pointer_cast<Material>(
            AZ::Interface<Physics::MaterialManager>::Get()->FindOrCreateMaterial(
                Physics::MaterialId::CreateRandom(),
                materialAsset));
    }
    
    Material::~Material()
    {
        AZ::Data::AssetBus::Handler::BusDisconnect();
    }

    Material::Material(const Physics::MaterialId& id, const AZ::Data::Asset<Physics::MaterialAsset>& materialAsset)
    {
        const MaterialConfiguration defaultMaterialConfiguration;

        // Create the JoltPhysicsMaterial with default values
        m_joltMaterial = JoltMaterialUniquePtr(
            new JoltPhysicsMaterial(
                "Default",
                JPH::Color::sWhite,
                defaultMaterialConfiguration.m_Friction, defaultMaterialConfiguration.m_restitution, defaultMaterialConfiguration.m_density
            ),
            [](JoltPhysics::JoltPhysicsMaterial* joltPhysicsMaterial)
            {
                // Nothing to do here yet
            });
        
        AZ_Assert(m_joltMaterial, "Failed to create Jolt material")
        m_joltMaterial->m_userData = this;
            
        // Assign default values to members
        m_friction = defaultMaterialConfiguration.m_Friction;
        m_restitution = defaultMaterialConfiguration.m_restitution;
        m_density = defaultMaterialConfiguration.m_density;
        m_debugColor = defaultMaterialConfiguration.m_debugColor;

        // When OnAssetReady is called, it will set all the properties from the material asset
        AZ::Data::AssetBus::Handler::BusConnect(m_materialAsset.GetId());
    }

    Physics::MaterialPropertyValue Material::GetProperty(AZStd::string_view propertyName) const
    {
        if (propertyName == MaterialConstants::FrictionName)
        {
            return GetFriction();
        }
        else if (propertyName == MaterialConstants::RestitutionName)
        {
            return GetRestitution();
        }
        else if (propertyName == MaterialConstants::DensityName)
        {
            return GetDensity();
        }
        else if (propertyName == MaterialConstants::RestitutionCombineModeName)
        {
            return static_cast<AZ::u32>(GetRestitutionCombineMode());
        }
        else if (propertyName == MaterialConstants::FrictionCombineModeName)
        {
            return static_cast<AZ::u32>(GetFrictionCombineMode());
        }
        else if (propertyName == MaterialConstants::DebugColorName)
        {
            return GetDebugColor();
        }
        else
        {
            AZ_Error("JoltPhysics::Material", false, "Unknown property '%.*s'", AZ_STRING_ARG(propertyName));
            return 0.0f;
        }
    }

    void Material::SetProperty(AZStd::string_view propertyName, Physics::MaterialPropertyValue value)
    {
        if (propertyName == MaterialConstants::FrictionName)
        {
            SetFriction(value.GetValue<float>());
        }
        else if (propertyName == MaterialConstants::RestitutionName)
        {
            SetRestitution(value.GetValue<float>());
        }
        else if (propertyName == MaterialConstants::DensityName)
        {
            SetDensity(value.GetValue<float>());
        }
        else if (propertyName == MaterialConstants::RestitutionCombineModeName)
        {
            SetRestitutionCombineMode(static_cast<CombineMode>(value.GetValue<AZ::u32>()));
        }
        else if (propertyName == MaterialConstants::FrictionCombineModeName)
        {
            SetFrictionCombineMode(static_cast<CombineMode>(value.GetValue<AZ::u32>()));
        }
        else if (propertyName == MaterialConstants::DebugColorName)
        {
            SetDebugColor(value.GetValue<AZ::Color>());
        }
        else
        {
            AZ_Error("JoltPhysics::Material", false, "Unknown property '%.*s'", AZ_STRING_ARG(propertyName));
        }
    }

    float Material::GetFriction() const
    {
        return m_density;
    }

    void Material::SetFriction(float friction)
    {
        AZ_Warning("Jolt Material", friction >= 0.0f, "Friction value %f is out of range, 0 will be used.", friction)
        m_joltMaterial->SetFriction(AZ::GetMax(0.0f, friction));
    }

    float Material::GetRestitution() const
    {
        return m_restitution;
    }

    void Material::SetRestitution(float restitution)
    {
        AZ_Warning(
            "Jolt Material", restitution >= 0.0f && restitution <= 1.0f, "Restitution value %f will be clamped into range [0, 1]",
            restitution)

        m_restitution = AZ::GetClamp(restitution, 0.0f, 1.0f);
        
        m_joltMaterial->SetRestitution(m_restitution);
    }

    float Material::GetDensity() const
    {
        return m_density;
    }

    void Material::SetDensity(float density)
    {
        AZ_Warning(
            "Jolt Material", density >= MaterialConstants::MinDensityLimit && density <= MaterialConstants::MaxDensityLimit,
            "Density value %f will be clamped into range [%f, %f].", density, MaterialConstants::MinDensityLimit, MaterialConstants::MaxDensityLimit);

        m_density = AZ::GetClamp(density, MaterialConstants::MinDensityLimit, MaterialConstants::MaxDensityLimit);

        m_joltMaterial->SetDensity(m_density);
    }

    CombineMode Material::GetFrictionCombineMode() const
    {
        return FromJoltCombineMode(m_joltMaterial->GetFrictionCombineMode());
    }

    void Material::SetFrictionCombineMode(CombineMode mode)
    {
        m_joltMaterial->SetFrictionCombineMode(ToJoltCombineMode(mode));
    }

    CombineMode Material::GetRestitutionCombineMode() const
    {
        return FromJoltCombineMode(m_joltMaterial->GetRestitutionCombineMode());
    }

    void Material::SetRestitutionCombineMode(CombineMode mode)
    {
        m_joltMaterial->SetRestitutionCombineMode(ToJoltCombineMode(mode));
    }

    const AZ::Color& Material::GetDebugColor() const
    {
        return m_debugColor;
    }

    void Material::SetDebugColor([[maybe_unused]] const AZ::Color& debugColor)
    {
    }

    const JoltPhysicsMaterial* Material::GetJoltMaterial() const
    {
        return m_joltMaterial.get();
    }

    void Material::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        m_materialAsset = asset;

        MaterialConfiguration::ValidateMaterialAsset(m_materialAsset);

        for (const auto& materialProperty : m_materialAsset->GetMaterialProperties())
        {
            SetProperty(materialProperty.first, materialProperty.second);
        }
    }

    void Material::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        OnAssetReady(asset);
    }
}
