#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/Asset/AssetManager.h>
#include <AzCore/Asset/AssetDataStream.h>
#include <AzCore/IO/FileIO.h>
#include <AzCore/IO/IOUtils.h>

#include <AzFramework/StringFunc/StringFunc.h>
#include <AzFramework/Physics/Material/PhysicsMaterialAsset.h>

#include <AssetBuilderSDK/SerializationDependencies.h>

#include <Editor/JoltEditorMaterialAsset.h>
#include <Editor/JoltEditorMaterialAssetBuilder.h>

namespace JoltPhysics
{
    void EditorMaterialAssetBuilder::CreateJobs(
        const AssetBuilderSDK::CreateJobsRequest& request,
        AssetBuilderSDK::CreateJobsResponse& response) const
    {
        for (const AssetBuilderSDK::PlatformInfo& platformInfo : request.m_enabledPlatforms)
        {
            AssetBuilderSDK::JobDescriptor jobDescriptor;
            jobDescriptor.m_critical = true;
            jobDescriptor.m_jobKey = "Jolt Material Asset";
            jobDescriptor.SetPlatformIdentifier(platformInfo.m_identifier.c_str());

            response.m_createJobOutputs.push_back(jobDescriptor);
        }

        response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
    }

    void EditorMaterialAssetBuilder::ProcessJob(
        const AssetBuilderSDK::ProcessJobRequest& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
        AZ::Data::Asset<EditorMaterialAsset> editorMaterialAsset = LoadEditorMaterialAsset(request.m_fullPath);
        if (!editorMaterialAsset)
        {
            AZ_Error("EditorMaterialAssetBuilder", false, "Failed to load Jolt EditorMaterialAsset: %s", request.m_fullPath.c_str());
            return;
        }

        AZ::Data::Asset<Physics::MaterialAsset> physicsMaterialAsset = editorMaterialAsset->GetMaterialConfiguration().CreateMaterialAsset();
        if (!physicsMaterialAsset.IsReady())
        {
            AZ_Error("EditorMaterialAssetBuilder", false, "Failed to create physics material assset.");
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        if (!SerializeOutPhysicsMaterialAsset(physicsMaterialAsset, request, response))
        {
            AZ_Error("EditorMaterialAssetBuilder", false, "Failed to serialize out physics material asset.");
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
    }

    AZ::Data::Asset<EditorMaterialAsset> EditorMaterialAssetBuilder::LoadEditorMaterialAsset(const AZStd::string& assetFullPath) const
    {
        auto assetDataStream = AZStd::make_shared<AZ::Data::AssetDataStream>();
        // Read in the data from a file to a buffer, then hand ownership of the buffer over to the assetDataStream
        {
            AZ::IO::FileIOStream stream(assetFullPath.c_str(), AZ::IO::OpenMode::ModeRead);
            if (!AZ::IO::RetryOpenStream(stream))
            {
                AZ_Error("EditorMaterialAssetBuilder", false, "Source file '%s' could not be opened.", assetFullPath.c_str());
                return {};
            }
            AZStd::vector<AZ::u8> fileBuffer(stream.GetLength());
            size_t bytesRead = stream.Read(fileBuffer.size(), fileBuffer.data());
            if (bytesRead != stream.GetLength())
            {
                AZ_Error("EditorMaterialAssetBuilder", false, "Source file '%s' could not be read.", assetFullPath.c_str());
                return {};
            }

            assetDataStream->Open(AZStd::move(fileBuffer));
        }

        AZ::Data::Asset<EditorMaterialAsset> JoltEditorMaterialAsset;
        JoltEditorMaterialAsset.Create(AZ::Data::AssetId(AZ::Uuid::CreateRandom()));

        auto* JoltEditorMaterialAssetHandler = AZ::Data::AssetManager::Instance().GetHandler(EditorMaterialAsset::RTTI_Type());
        if (!JoltEditorMaterialAssetHandler)
        {
            AZ_Error("EditorMaterialAssetBuilder", false, "Unable to find Jolt EditorMaterialAsset handler.");
            return {};
        }

        if (JoltEditorMaterialAssetHandler->LoadAssetDataFromStream(JoltEditorMaterialAsset, assetDataStream, nullptr) != AZ::Data::AssetHandler::LoadResult::LoadComplete)
        {
            AZ_Error("EditorMaterialAssetBuilder", false, "Failed to load Jolt EditorMaterialAsset: '%s'", assetFullPath.c_str());
            return {};
        }

        if (!JoltEditorMaterialAsset)
        {
            AZ_Error("EditorMaterialAssetBuilder", false, "Jolt EditorMaterialAsset loaded with invalid data: '%s'", assetFullPath.c_str());
            return {};
        }

        return JoltEditorMaterialAsset;
    }

    bool EditorMaterialAssetBuilder::SerializeOutPhysicsMaterialAsset(
        AZ::Data::Asset<Physics::MaterialAsset> physicsMaterialAsset,
        const AssetBuilderSDK::ProcessJobRequest& request,
        AssetBuilderSDK::ProcessJobResponse& response) const
    {
        AZStd::string physicsMaterialFilename = request.m_sourceFile;
        AzFramework::StringFunc::Path::ReplaceExtension(physicsMaterialFilename, Physics::MaterialAsset::FileExtension);

        AZStd::string physicsMaterialAssetOutputPath;
        AzFramework::StringFunc::Path::ConstructFull(request.m_tempDirPath.c_str(), physicsMaterialFilename.c_str(), physicsMaterialAssetOutputPath, true);

        if (!AZ::Utils::SaveObjectToFile(physicsMaterialAssetOutputPath, AZ::DataStream::ST_XML, physicsMaterialAsset.Get()))
        {
            AZ_Error("EditorMaterialAssetBuilder", false, "Failed to save physics material asset to file: %s", physicsMaterialAssetOutputPath.c_str());
            return false;
        }

        AssetBuilderSDK::JobProduct physicsMaterialJobProduct;
        if (!AssetBuilderSDK::OutputObject(
            physicsMaterialAsset.Get(),
            physicsMaterialAssetOutputPath,
            azrtti_typeid<Physics::MaterialAsset>(),
            Physics::MaterialAsset::AssetSubId,
            physicsMaterialJobProduct))
        {
            AZ_Error("EditorMaterialAssetBuilder", false, "Failed to output product dependencies.");
            return false;
        }

        response.m_outputProducts.push_back(AZStd::move(physicsMaterialJobProduct));
        return true;
    }
} // JoltPhysics
