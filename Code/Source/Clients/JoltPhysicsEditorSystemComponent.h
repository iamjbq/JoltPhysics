
#pragma once

#include <AzCore/Asset/AssetManager.h>
#include <AzCore/Component/Component.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzFramework/Physics/Common/PhysicsEvents.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/ActionManager/ActionManagerRegistrationNotificationBus.h>
#include <AzToolsFramework/Entity/EditorEntityContextBus.h>

#include <Editor/JoltEditorMaterialAssetBuilder.h>
#include <Clients/JoltPhysicsSystemComponent.h>

namespace JoltPhysics
{
    /// System component for JoltPhysics editor
    class JoltPhysicsEditorSystemComponent
        : public JoltPhysicsSystemComponent
        , public Physics::EditorWorldBus::Handler
        , private AzToolsFramework::EditorEntityContextNotificationBus::Handler
        , protected AzToolsFramework::EditorEvents::Bus::Handler
        , public AzToolsFramework::ActionManagerRegistrationNotificationBus::Handler
    {
        using BaseSystemComponent = JoltPhysicsSystemComponent;
    public:
        AZ_COMPONENT_DECL(JoltPhysicsEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        JoltPhysicsEditorSystemComponent() = default;
        JoltPhysicsEditorSystemComponent(const JoltPhysicsEditorSystemComponent&) = delete;
        JoltPhysicsEditorSystemComponent& operator=(const JoltPhysicsEditorSystemComponent&) = delete;

        // Physics::EditorWorldBus overrides...
        AzPhysics::SceneHandle GetEditorSceneHandle() const override;

        // ActionManagerRegistrationNotificationBus overrides ...
        void OnActionRegistrationHook() override;
        void OnActionContextModeBindingHook() override;
        void OnMenuBindingHook() override;

    protected:
        // AztoolsFramework::EditorEvents overrides...
        void NotifyRegisterViews() override;

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        // AzToolsFramework::EditorEntityContextNotificationBus overrides...
        void OnStartPlayInEditorBegin() override;
        void OnStopPlayInEditor() override;

        AzPhysics::SceneHandle m_editorWorldSceneHandle = AzPhysics::InvalidSceneHandle;

        // Assets related data
        AZStd::vector<AZStd::unique_ptr<AZ::Data::AssetHandler>> m_assetHandlers;

        // Asset builder for Jolt material asset
        EditorMaterialAssetBuilder m_materialAssetBuilder;
    };
} // namespace JoltPhysics
