
#include "EditorRigidBodyComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace JoltPhysics
{
    AZ_COMPONENT_IMPL(EditorRigidBodyComponent, "EditorRigidBodyComponent", "{149D1675-B175-4400-AA35-49E45A57F434}");

    void EditorRigidBodyComponent::Activate()
    {
    }

    void EditorRigidBodyComponent::Deactivate()
    {
    }

    void EditorRigidBodyComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorRigidBodyComponent, AZ::Component>()
                ->Version(1)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<EditorRigidBodyComponent>("JoltEditorRigidBodyComponent", "[Description of functionality provided by this component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "ComponentCategory")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<EditorRigidBodyComponent>("JoltEditorRigidBody Component Group")
                ->Attribute(AZ::Script::Attributes::Category, "JoltPhysics Gem Group")
                ;
        }
    }

    void EditorRigidBodyComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("JoltEditorRigidBodyComponentService"));
    }

    void EditorRigidBodyComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void EditorRigidBodyComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void EditorRigidBodyComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }
} // namespace JoltPhysics
