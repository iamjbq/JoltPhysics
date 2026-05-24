#pragma once

#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>

namespace JoltPhysics
{
    //! Class for in-editor Jolt Static Rigid Body Component.
    class EditorStaticRigidBodyComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , public AZ::EntityBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(
            EditorStaticRigidBodyComponent, "{EC28A5BF-890E-48DC-879B-84C1B2ADBC95}", AzToolsFramework::Components::EditorComponentBase);
        static void Reflect(AZ::ReflectContext* context);

        EditorStaticRigidBodyComponent() = default;
        ~EditorStaticRigidBodyComponent() = default;

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        
        void Activate() override;
        void Deactivate() override;
        
        // AZ::EntityBus overrides ...
        void OnEntityActivated(const AZ::EntityId& entityId) override;

        // EditorComponentBase
        void BuildGameEntity(AZ::Entity* gameEntity) override;
        
    private:
        RigidBodyConfiguration m_joltSpecificConfig; //!< Properties specific to Jolt which might not have exact equivalents in other physics engines.
        AzPhysics::SimulatedBodyHandle m_editorRigidBodyHandle = AzPhysics::InvalidSimulatedBodyHandle;
        AzPhysics::SceneHandle m_editorSceneHandle = AzPhysics::InvalidSceneHandle;
        
        AzPhysics::SystemEvents::OnConfigurationChangedEvent::Handler m_joltConfigChangedHandler;
        AzPhysics::SystemEvents::OnDefaultSceneConfigurationChangedEvent::Handler m_sceneConfigChangedHandler; //!< Responds to changes in Scene Config.
    };
} // namespace JoltPhysics
