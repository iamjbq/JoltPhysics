
#include <AzCore/Interface/Interface.h>
#include <AzCore/IO/Path/Path.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Settings/SettingsRegistryMergeUtils.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Collision/CollisionEvents.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>

#include <Tools/JoltPhysicsEditorSystemComponent.h>
#include <Editor/JoltEditorMaterialAsset.h>
#include <JoltPhysics/JoltPhysicsTypeIds.h>
#include <System/JoltSystem.h>

namespace JoltPhysics
{
    AZ_COMPONENT_IMPL(JoltPhysicsEditorSystemComponent, "JoltPhysicsEditorSystemComponent",
        JoltPhysicsEditorSystemComponentTypeId, BaseSystemComponent);

    void JoltPhysicsEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        EditorMaterialAsset::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<JoltPhysicsEditorSystemComponent, AZ::Component>()
                ->Version(1)
                ->Attribute(AZ::Edit::Attributes::SystemComponentTags, AZStd::vector<AZ::Crc32>({ AZ_CRC_CE("AssetBuilder") }))
                ;
        }
    }


    void JoltPhysicsEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("PhysicsEditorService"));
    }

    void JoltPhysicsEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("PhysicsEditorService"));
    }

    void JoltPhysicsEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("PhysicsService"));
    }

    void JoltPhysicsEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("AssetDatabaseService"));
        dependent.push_back(AZ_CRC_CE("AssetCatalogService"));
        dependent.push_back(AZ_CRC_CE("PhysicsMaterialService"));
    }

    void JoltPhysicsEditorSystemComponent::Activate()
    {
        Physics::EditorWorldBus::Handler::BusConnect();

        // Register Jolt Material Asset
        auto* materialAsset = aznew AzFramework::GenericAssetHandler<JoltPhysics::EditorMaterialAsset>("Jolt Material", Physics::MaterialAsset::AssetGroup, EditorMaterialAsset::FileExtension);
        materialAsset->Register();
        m_assetHandlers.emplace_back(materialAsset);

        // Register Jolt Material Asset Builder
        AssetBuilderSDK::AssetBuilderDesc materialAssetBuilderDescriptor;
        materialAssetBuilderDescriptor.m_name = "Jolt Material Asset Builder";
        materialAssetBuilderDescriptor.m_version = 1; // bump this to rebuild all joltmaterial files
        materialAssetBuilderDescriptor.m_patterns.push_back(AssetBuilderSDK::AssetBuilderPattern(AZStd::string::format("*.%s", EditorMaterialAsset::FileExtension), AssetBuilderSDK::AssetBuilderPattern::PatternType::Wildcard));
        materialAssetBuilderDescriptor.m_busId = azrtti_typeid<EditorMaterialAssetBuilder>();
        materialAssetBuilderDescriptor.m_createJobFunction = [this](const AssetBuilderSDK::CreateJobsRequest& request, AssetBuilderSDK::CreateJobsResponse& response)
        {
            m_materialAssetBuilder.CreateJobs(request, response);
        };
        materialAssetBuilderDescriptor.m_processJobFunction = [this](const AssetBuilderSDK::ProcessJobRequest& request, AssetBuilderSDK::ProcessJobResponse& response)
        {
            m_materialAssetBuilder.ProcessJob(request, response);
        };
        m_materialAssetBuilder.BusConnect(materialAssetBuilderDescriptor.m_busId);
        AssetBuilderSDK::AssetBuilderBus::Broadcast(&AssetBuilderSDK::AssetBuilderBus::Handler::RegisterBuilderInformation, materialAssetBuilderDescriptor);

        if (auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get())
        {
            AzPhysics::SceneConfiguration editorWorldConfiguration = physicsSystem->GetDefaultSceneConfiguration();
            editorWorldConfiguration.m_sceneName = AzPhysics::EditorPhysicsSceneName;
            m_editorWorldSceneHandle = physicsSystem->AddScene(editorWorldConfiguration);
        }


        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
        AzToolsFramework::EditorEntityContextNotificationBus::Handler::BusConnect();
        AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler::BusConnect();
    }

    void JoltPhysicsEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler::BusDisconnect();
        AzToolsFramework::EditorEntityContextNotificationBus::Handler::BusDisconnect();
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        Physics::EditorWorldBus::Handler::BusDisconnect();

        if (auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get())
        {
            physicsSystem->RemoveScene(m_editorWorldSceneHandle);
        }
        m_editorWorldSceneHandle = AzPhysics::InvalidSceneHandle;

        m_materialAssetBuilder.BusDisconnect();

        for (auto& assetHandler : m_assetHandlers)
        {
            if (auto editorMaterialAssetHandler = azrtti_cast<AzFramework::GenericAssetHandler<JoltPhysics::EditorMaterialAsset>*>(assetHandler.get());
                editorMaterialAssetHandler != nullptr)
            {
                editorMaterialAssetHandler->Unregister();
            }
        }
        m_assetHandlers.clear();
    }

    AzPhysics::SceneHandle JoltPhysicsEditorSystemComponent::GetEditorSceneHandle() const
    {
        return m_editorWorldSceneHandle;
    }

    void JoltPhysicsEditorSystemComponent::OnActionRegistrationHook()
    {
    }

    void JoltPhysicsEditorSystemComponent::OnActionContextModeBindingHook()
    {
    }

    void JoltPhysicsEditorSystemComponent::OnMenuBindingHook()
    {
    }

    void JoltPhysicsEditorSystemComponent::NotifyRegisterViews()
    {
    }

    void JoltPhysicsEditorSystemComponent::OnStartPlayInEditorBegin()
    {
        if (auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get())
        {
            if (AzPhysics::Scene* scene = physicsSystem->GetScene(m_editorWorldSceneHandle))
            {
                scene->SetEnabled(false);
            }
        }
    }

    void JoltPhysicsEditorSystemComponent::OnStopPlayInEditor()
    {
        if (auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get())
        {
            if (AzPhysics::Scene* scene = physicsSystem->GetScene(m_editorWorldSceneHandle))
            {
                scene->SetEnabled(true);
            }
        }
    }
} // namespace JoltPhysics
