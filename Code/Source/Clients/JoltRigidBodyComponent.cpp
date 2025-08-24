
#include "JoltRigidBodyComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace JoltPhysics
{
    AZ_COMPONENT_IMPL(JoltRigidBodyComponent, "JoltRigidBodyComponent", "{281F429A-4D56-4391-9E81-A7F696006061}");

    void JoltRigidBodyComponent::Activate()
    {
        JoltRigidBodyRequestBus::Handler::BusConnect(GetEntityId());
    }

    void JoltRigidBodyComponent::Deactivate()
    {
        JoltRigidBodyRequestBus::Handler::BusDisconnect(GetEntityId());
    }

    void JoltRigidBodyComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<JoltRigidBodyComponent, AZ::Component>()
                ->Version(1)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<JoltRigidBodyComponent>("JoltRigidBodyComponent", "[Description of functionality provided by this component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "ComponentCategory")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<JoltRigidBodyComponent>("JoltRigidBody Component Group")
                ->Attribute(AZ::Script::Attributes::Category, "JoltPhysics Gem Group")
                ;
        }
    }

    void JoltRigidBodyComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("JoltRigidBodyComponentService"));
    }

    void JoltRigidBodyComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void JoltRigidBodyComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void JoltRigidBodyComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }
} // namespace JoltPhysics
