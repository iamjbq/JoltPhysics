
#include <AzCore/Interface/Interface.h>
#include <AzCore/IO/Path/Path.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Settings/SettingsRegistryMergeUtils.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Collision/CollisionEvents.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>

#include "JoltPhysicsEditorSystemComponent.h"
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
            serializeContext->Class<JoltPhysicsEditorSystemComponent, JoltPhysicsSystemComponent>()
                ->Version(1);
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
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("JoltPhysicsEditorService"));
    }

    void JoltPhysicsEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("JoltPhysicsEditorService"));
    }

    void JoltPhysicsEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void JoltPhysicsEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void JoltPhysicsEditorSystemComponent::Activate()
    {
        if (auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get())
        {
            AzPhysics::SceneConfiguration editorWorldConfiguration = physicsSystem->GetDefaultSceneConfiguration();
            editorWorldConfiguration.m_sceneName = AzPhysics::EditorPhysicsSceneName;
            m_editorWorldSceneHandle = physicsSystem->AddScene(editorWorldConfiguration);
        }

        // JoltPhysicsSystemComponent::Activate();

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

        // JoltPhysicsSystemComponent::Deactivate();
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
