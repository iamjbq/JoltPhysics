
#include <AzCore/Interface/Interface.h>
#include <AzCore/IO/Path/Path.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Settings/SettingsRegistryMergeUtils.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Collision/CollisionEvents.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>

#include <Tools/JoltPhysicsEditorSystemComponent.h>
#include <JoltPhysics/JoltPhysicsTypeIds.h>
#include <System/JoltSystem.h>

namespace JoltPhysics
{
    AZ_COMPONENT_IMPL(JoltPhysicsEditorSystemComponent, "JoltPhysicsEditorSystemComponent",
        JoltPhysicsEditorSystemComponentTypeId, BaseSystemComponent);

    void JoltPhysicsEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<JoltPhysicsEditorSystemComponent, AZ::Component>()
                ->Version(1)
                ->Attribute(AZ::Edit::Attributes::SystemComponentTags, AZStd::vector<AZ::Crc32>({ AZ_CRC_CE("AssetBuilder") }))
                ;
        }
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
    }

    void JoltPhysicsEditorSystemComponent::Activate()
    {
        if (auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get())
        {
            AzPhysics::SceneConfiguration editorWorldConfiguration = physicsSystem->GetDefaultSceneConfiguration();
            editorWorldConfiguration.m_sceneName = AzPhysics::EditorPhysicsSceneName;
            m_editorWorldSceneHandle = physicsSystem->AddScene(editorWorldConfiguration);
        }

        AZ_TracePrintf("EditorSystemComponent", "Editor system component activated\n")

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
