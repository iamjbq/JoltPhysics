
#include <Clients/EditorRigidBodyComponent.h>
#include <Clients/RigidBodyComponent.h>

#include <AzCore/Interface/Interface.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Physics/Utils.h>
#include <AzFramework/Physics/NameConstants.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
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

    void EditorRigidBodyComponent::BuildGameEntity([[maybe_unused]] AZ::Entity* gameEntity)
    {
        gameEntity->CreateComponent<JoltPhysics::RigidBodyComponent>(m_config, m_joltSpecificConfig, AzPhysics::InvalidSceneHandle);
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

} // namespace JoltPhysics
