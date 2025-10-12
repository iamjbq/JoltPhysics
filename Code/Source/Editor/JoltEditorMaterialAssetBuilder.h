#pragma once

#include <AssetBuilderSDK/AssetBuilderBusses.h>
#include <AssetBuilderSDK/AssetBuilderSDK.h>

#include <JoltPhysics/Material/JoltMaterialConfiguration.h>

namespace JoltPhysics
{
    class EditorMaterialAsset;

    //! Builder to convert Jolt Editor Material assets in the
    //! source folder into Physics Material assets in the cache folder.
    class EditorMaterialAssetBuilder
        : public AssetBuilderSDK::AssetBuilderCommandBus::Handler
    {
    public:
        AZ_RTTI(JoltPhysics::EditorMaterialAssetBuilder, "{24A3C108-63D8-4C01-B45C-6A9A8EA5C969}");

        EditorMaterialAssetBuilder() = default;

        // Asset Builder Callback Functions...
        void CreateJobs(const AssetBuilderSDK::CreateJobsRequest& request, AssetBuilderSDK::CreateJobsResponse& response) const;
        void ProcessJob(const AssetBuilderSDK::ProcessJobRequest& request, AssetBuilderSDK::ProcessJobResponse& response) const;

        // AssetBuilderSDK::AssetBuilderCommandBus overrides...
        void ShutDown() override { }

    private:
        AZ::Data::Asset<EditorMaterialAsset> LoadEditorMaterialAsset(const AZStd::string& assetFullPath) const;

        bool SerializeOutPhysicsMaterialAsset(
            AZ::Data::Asset<Physics::MaterialAsset> physicsMaterialAsset,
            const AssetBuilderSDK::ProcessJobRequest& request,
            AssetBuilderSDK::ProcessJobResponse& response) const;
    };
} // JoltPhysics

