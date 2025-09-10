
#include <AzCore/Math/Transform.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/std/containers/vector.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
#include <AzFramework/Physics/Configuration/SceneConfiguration.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Utils.h>

#include <Clients/RigidBodyComponent.h>
#include <Clients/RigidBody.h>
#include <Clients/Shape.h>

namespace JoltPhysics
{
    void RigidBodyComponent::Activate()
    {
        JoltRigidBodyRequestBus::Handler::BusConnect(GetEntityId());
    }

    void RigidBodyComponent::Deactivate()
    {
        JoltRigidBodyRequestBus::Handler::BusDisconnect(GetEntityId());
    }

    void RigidBodyComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {

    }

    int RigidBodyComponent::GetTickOrder()
    {
        return AZ::ComponentTickBus::TICK_PHYSICS;
    }

    void RigidBodyComponent::SetupConfiguration()
    {
        // Get necessary information from the components and fill up the configuration structure
        auto entityId = GetEntityId();

        AZ::Transform newTransform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(newTransform, entityId, &AZ::TransformInterface::GetWorldTM);
        m_configuration.m_position = newTransform.GetTranslation();
        m_configuration.m_orientation = newTransform.GetRotation();
        m_configuration.m_entityId = entityId;
        m_configuration.m_debugName = GetEntity()->GetName();
    }

    void RigidBodyComponent::CreateRigidBody()
    {
        // BodyConfigurationComponentBus::EventResult(m_configuration, GetEntityId(), &BodyConfigurationComponentRequests::GetRigidBodyConfiguration);

        // Create rigid body
        SetupConfiguration();
        // Add shapes
        AZStd::vector<AZStd::shared_ptr<Physics::Shape>> shapes;
        // ColliderComponentRequestBus::EnumerateHandlersId(GetEntityId(), [&shapes](ColliderComponentRequests* handler)
        //     {
        //         AZStd::vector<AZStd::shared_ptr<Physics::Shape>> newShapes = handler->GetShapes();
        //         shapes.insert(shapes.end(), newShapes.begin(), newShapes.end());
        //         return true;
        //     });
        m_configuration.m_colliderAndShapeData = shapes;

        if (m_cachedSceneInterface != nullptr)
        {
            m_configuration.m_startSimulationEnabled = false; //enable physics will enable this when called.
            m_rigidBodyHandle = m_cachedSceneInterface->AddSimulatedBody(m_attachedSceneHandle, &m_configuration);
            ApplyJoltSpecificConfiguration();

            // Listen to the Jolt system for events concerning this entity.
            AzPhysics::Scene* scene = m_cachedSceneInterface->GetScene(m_attachedSceneHandle);

            if (scene && scene->GetConfiguration().m_enableActiveActors)
            {
                AzPhysics::SimulatedBody* body =
                    m_cachedSceneInterface->GetSimulatedBodyFromHandle(m_attachedSceneHandle, m_rigidBodyHandle);
                body->RegisterOnSyncTransformHandler(m_activeBodySyncTransformHandler);
            }
            else
            {
                m_cachedSceneInterface->RegisterSceneSimulationFinishHandler(m_attachedSceneHandle, m_sceneFinishSimHandler);
            }
        }

        // TODO: Check if Jolt does this internally
        if (m_configuration.m_interpolateMotion)
        {
            AZ::TickBus::Handler::BusConnect();
        }

        AZ::TransformNotificationBus::MultiHandler::BusConnect(GetEntityId());
        Physics::RigidBodyRequestBus::Handler::BusConnect(GetEntityId());
        AzPhysics::SimulatedBodyComponentRequestsBus::Handler::BusConnect(GetEntityId());
    }

    void RigidBodyComponent::DestroyRigidBody()
    {
        if (m_cachedSceneInterface)
        {
            m_cachedSceneInterface->RemoveSimulatedBody(m_attachedSceneHandle, m_rigidBodyHandle);
            m_rigidBodyHandle = AzPhysics::InvalidSimulatedBodyHandle;
        }

        Physics::RigidBodyRequestBus::Handler::BusDisconnect();
        AzPhysics::SimulatedBodyComponentRequestsBus::Handler::BusDisconnect();
        AZ::TransformNotificationBus::MultiHandler::BusDisconnect();
        m_sceneFinishSimHandler.Disconnect();
        m_activeBodySyncTransformHandler.Disconnect();
        AZ::TickBus::Handler::BusDisconnect();

        m_isLastMovementFromKinematicSource = false;
        m_rigidBodyTransformNeedsUpdateOnPhysReEnable = false;
    }

    void RigidBodyComponent::ApplyJoltSpecificConfiguration()
    {

    }

    void RigidBodyComponent::InitPhysicsTickHandler()
    {

    }


    void RigidBodyComponent::Reflect(AZ::ReflectContext* context)
    {
        RigidBodyConfiguration::Reflect(context);
        RigidBody::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RigidBodyComponent, AZ::Component>()
                ->Version(1)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RigidBodyComponent>("RigidBodyComponent", "[Description of functionality provided by this component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "ComponentCategory")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ;
            }
        }

        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<RigidBodyComponent>("JoltRigidBody Component Group")
                ->Attribute(AZ::Script::Attributes::Category, "JoltPhysics Gem Group")
                ;
        }
    }

    RigidBodyComponent::RigidBodyComponent()
    {
        InitPhysicsTickHandler();
    }

    RigidBodyComponent::RigidBodyComponent(const AzPhysics::RigidBodyConfiguration& config, AzPhysics::SceneHandle sceneHandle)
        : m_configuration(config)
        , m_attachedSceneHandle(sceneHandle)
    {
        InitPhysicsTickHandler();
    }

    RigidBodyComponent::RigidBodyComponent(
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
