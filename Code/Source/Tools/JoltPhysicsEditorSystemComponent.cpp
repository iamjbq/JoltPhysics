
#include <AzCore/Serialization/SerializeContext.h>
#include "JoltPhysicsEditorSystemComponent.h"

#include <JoltPhysics/JoltPhysicsTypeIds.h>

namespace JoltPhysics
{
    AZ_COMPONENT_IMPL(JoltPhysicsEditorSystemComponent, "JoltPhysicsEditorSystemComponent",
        JoltPhysicsEditorSystemComponentTypeId, BaseSystemComponent);

    void JoltPhysicsEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<JoltPhysicsEditorSystemComponent, JoltPhysicsSystemComponent>()
                ->Version(0);
        }
    }

    JoltPhysicsEditorSystemComponent::JoltPhysicsEditorSystemComponent() = default;

    JoltPhysicsEditorSystemComponent::~JoltPhysicsEditorSystemComponent() = default;

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
        JoltPhysicsSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void JoltPhysicsEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        JoltPhysicsSystemComponent::Deactivate();
    }

} // namespace JoltPhysics
