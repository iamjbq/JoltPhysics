
#include <AzCore/Math/Transform.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/std/containers/vector.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
#include <AzFramework/Physics/Configuration/SceneConfiguration.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Utils.h>

#include <Clients/JoltRigidBodyComponent.h>
#include <Clients/RigidBody.h>
#include <Clients/Shape.h>

namespace JoltPhysics
{
    void JoltRigidBodyComponent::Activate()
    {
        JoltRigidBodyRequestBus::Handler::BusConnect(GetEntityId());
    }

    void JoltRigidBodyComponent::Deactivate()
    {
        JoltRigidBodyRequestBus::Handler::BusDisconnect(GetEntityId());
    }

    void JoltRigidBodyComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {

    }

    int JoltRigidBodyComponent::GetTickOrder()
    {
        return AZ::ComponentTickBus::TICK_PHYSICS;
    }

    void JoltRigidBodyComponent::InitPhysicsTickHandler()
    {

    }


    void JoltRigidBodyComponent::Reflect(AZ::ReflectContext* context)
    {
        RigidBodyConfiguration::Reflect(context);
        RigidBody::Reflect(context);

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

    JoltRigidBodyComponent::JoltRigidBodyComponent()
    {
        InitPhysicsTickHandler();
    }

    JoltRigidBodyComponent::JoltRigidBodyComponent(const AzPhysics::RigidBodyConfiguration& config, AzPhysics::SceneHandle sceneHandle)
        : m_configuration(config)
        , m_attachedSceneHandle(sceneHandle)
    {
        InitPhysicsTickHandler();
    }

    JoltRigidBodyComponent::JoltRigidBodyComponent(
        const AzPhysics::RigidBodyConfiguration& baseConfig,
        const RigidBodyConfiguration& joltSpecificConfig,
        AzPhysics::SceneHandle sceneHandle)
        : m_configuration(baseConfig)
        , m_joltSpecificConfiguration(joltSpecificConfig)
        , m_attachedSceneHandle(sceneHandle)
    {
        InitPhysicsTickHandler();
    }
} // namespace JoltPhysics
