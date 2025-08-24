
#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/JoltPhysicsSystemComponent.h>

namespace JoltPhysics
{
    /// System component for JoltPhysics editor
    class JoltPhysicsEditorSystemComponent
        : public JoltPhysicsSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = JoltPhysicsSystemComponent;
    public:
        AZ_COMPONENT_DECL(JoltPhysicsEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        JoltPhysicsEditorSystemComponent();
        ~JoltPhysicsEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace JoltPhysics
