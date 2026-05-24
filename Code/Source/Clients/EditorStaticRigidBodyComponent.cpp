#include <AzCore/Serialization/EditContext.h>

#include <AzFramework/Physics/NameConstants.h>

#include <Clients/EditorStaticRigidBodyComponent.h>
#include <Clients/StaticRigidBodyComponent.h>

namespace JoltPhysics
{
    void EditorStaticRigidBodyComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorStaticRigidBodyComponent, AzToolsFramework::Components::EditorComponentBase>()
                ->Version(1)
                ->Field("JoltSpecificConfiguration", &EditorStaticRigidBodyComponent::m_joltSpecificConfig);

            if (auto* editContext = serializeContext->GetEditContext())
            {
                constexpr const char* ToolTip = "The entity behaves as a non-movable rigid body in Jolt.";
                constexpr const char* HelpPageURL = "https://jrouwe.github.io/JoltPhysics/class_body.html";

                editContext->Class<EditorStaticRigidBodyComponent>("Static Rigid Body", ToolTip)
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Jolt")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/PhysXStaticRigidBody.svg")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Icons/Components/Viewport/PhysXStaticRigidBody.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                        ->Attribute(AZ::Edit::Attributes::HelpPageURL, HelpPageURL)
                        // ->UIElement(AZ::Edit::UIHandlers::Label, "", ToolTip)
                        // ->Attribute(AZ::Edit::Attributes::NameLabelOverride, "")
                        // ->Attribute(
                        //     AZ::Edit::Attributes::ValueText,
                        //     Physics::NameConstants::GetBulletPoint() + " This component is empty and has no modifiable properties.<br>" +
                        //     Physics::NameConstants::GetBulletPoint() + " Its purpose is to be a non-movable rigid body in Jolt. <a href=\"" + HelpPageURL + "\">Read more</a>")
                        ->DataElement(
                            AZ::Edit::UIHandlers::Default,
                            &EditorStaticRigidBodyComponent::m_joltSpecificConfig,
                            "Jolt-Specific Configuration",
                            "Settings which are specific to Jolt, rather than generic.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly);
            }
        }
    }

    void EditorStaticRigidBodyComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("PhysicsWorldBodyService"));
        provided.push_back(AZ_CRC_CE("PhysicsRigidBodyService"));
        provided.push_back(AZ_CRC_CE("PhysicsStaticRigidBodyService"));
    }

    void EditorStaticRigidBodyComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("PhysicsRigidBodyService"));
    }

    void EditorStaticRigidBodyComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("TransformService"));
        required.push_back(AZ_CRC_CE("PhysicsColliderService"));
    }

    void EditorStaticRigidBodyComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("NonUniformScaleService"));
    }

    void EditorStaticRigidBodyComponent::Activate()
    {
        m_joltSpecificConfig.m_colliderConfig.SetPropertyVisibility(Physics::ColliderConfiguration::PropertyVisibility::MaterialSelection, false);
        m_joltSpecificConfig.m_colliderConfig.SetPropertyVisibility(Physics::ColliderConfiguration::PropertyVisibility::Offset, false);
        m_joltSpecificConfig.m_colliderConfig.SetPropertyVisibility(Physics::ColliderConfiguration::PropertyVisibility::Tag, false);
        m_joltSpecificConfig.m_colliderConfig.SetPropertyVisibility(Physics::ColliderConfiguration::PropertyVisibility::ContactOffset, false);
        m_joltSpecificConfig.SetPropertyVisibility(RigidBodyConfiguration::SolverIteration, false);
        m_joltSpecificConfig.SetPropertyVisibility(RigidBodyConfiguration::CanSleep, false);
        
        m_joltConfigChangedHandler = AzPhysics::SystemEvents::OnConfigurationChangedEvent::Handler(
            []([[maybe_unused]] const AzPhysics::SystemConfiguration* config)
            {
                AzToolsFramework::PropertyEditorGUIMessages::Bus::Broadcast(&AzToolsFramework::PropertyEditorGUIMessages::RequestRefresh,
                    AzToolsFramework::PropertyModificationRefreshLevel::Refresh_AttributesAndValues);
            });
        
        if (auto* joltSystem = GetJoltSystem())
        {
            joltSystem->RegisterSystemConfigurationChangedEvent(m_joltConfigChangedHandler);
        }
        
        // During activation all the editor collider components will create their physics shapes.
        // Delaying the creation of the editor dynamic rigid body to OnEntityActivated so all the shapes are ready.
        AZ::EntityBus::Handler::BusConnect(GetEntityId());
    }

    void EditorStaticRigidBodyComponent::Deactivate()
    {
        AZ::EntityBus::Handler::BusDisconnect();

        m_joltConfigChangedHandler.Disconnect();
        m_sceneConfigChangedHandler.Disconnect();
    }

    void EditorStaticRigidBodyComponent::OnEntityActivated([[maybe_unused]] const AZ::EntityId& entityId)
    {
        AZ::EntityBus::Handler::BusDisconnect();
        
        m_sceneConfigChangedHandler = AzPhysics::SystemEvents::OnDefaultSceneConfigurationChangedEvent::Handler(
            [this]([[maybe_unused]] const AzPhysics::SceneConfiguration* config)
            {
                this->InvalidatePropertyDisplay(AzToolsFramework::Refresh_EntireTree);
            });

        if (auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get())
        {
            physicsSystem->RegisterOnDefaultSceneConfigurationChangedEventHandler(m_sceneConfigChangedHandler);
        }
    }

    void EditorStaticRigidBodyComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        gameEntity->CreateComponent<StaticRigidBodyComponent>();
    }
}
