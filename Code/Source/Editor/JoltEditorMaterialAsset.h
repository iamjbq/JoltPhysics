#pragma once

#include <JoltPhysics/Material/JoltMaterialConfiguration.h>

namespace JoltPhysics
{
    //! EditorMaterialAsset defines a single Jolt material asset.
    //! This is an editor asset, and it's authored by Asset Editor.
    //! When this asset is processed by Asset Processor it creates
    //! a generic Physics material asset in the cache (agnostic to backend).
    class EditorMaterialAsset
        : public AZ::Data::AssetData
    {
    public:
        AZ_CLASS_ALLOCATOR(JoltPhysics::EditorMaterialAsset, AZ::SystemAllocator);
        AZ_RTTI(JoltPhysics::EditorMaterialAsset, "{0A3F0A31-3271-42FE-A0F7-627C5E36A023}", AZ::Data::AssetData);

        static void Reflect(AZ::ReflectContext* context);

        static constexpr const char* FileExtension = "joltmaterial";

        EditorMaterialAsset() = default;
        virtual ~EditorMaterialAsset() = default;

        const MaterialConfiguration& GetMaterialConfiguration() const;

    protected:
        MaterialConfiguration m_materialConfiguration;
    };
} // JoltPhysics
