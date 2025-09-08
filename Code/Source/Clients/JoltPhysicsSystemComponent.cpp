
#include "JoltPhysicsSystemComponent.h"

#include <AzCore/Serialization/SerializeContext.h>

#include <System/JoltSystem.h>
#include <JoltPhysics/Configuration/JoltConfiguration.h>
#include <JoltPhysics/JoltPhysicsTypeIds.h>

namespace JoltPhysics
{
    AZ_COMPONENT_IMPL(JoltPhysicsSystemComponent, "JoltPhysicsSystemComponent",
        JoltPhysicsSystemComponentTypeId);

    void JoltPhysicsSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<JoltPhysicsSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void JoltPhysicsSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("JoltPhysicsService"));
    }

    void JoltPhysicsSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("JoltPhysicsService"));
    }

    void JoltPhysicsSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void JoltPhysicsSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    JoltPhysicsSystemComponent::JoltPhysicsSystemComponent()
    {
    }

    JoltPhysicsSystemComponent::~JoltPhysicsSystemComponent()
    {
    }

    void JoltPhysicsSystemComponent::Init()
    {
    }

    void JoltPhysicsSystemComponent::Activate()
    {
        JoltPhysicsRequestBus::Handler::BusConnect();

        ActivateSimulation();
    }

    void JoltPhysicsSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        JoltPhysicsRequestBus::Handler::BusDisconnect();
    }

    void JoltPhysicsSystemComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_joltSystem)
        {
            m_joltSystem->Simulate(deltaTime);
        }
    }

    int JoltPhysicsSystemComponent::GetTickOrder()
    {
        return AZ::ComponentTickBus::TICK_PHYSICS_SYSTEM;
    }

    void JoltPhysicsSystemComponent::EnableAutoManagedPhysicsTick(bool shouldTick)
    {
        if (shouldTick && !m_isTickingPhysics)
        {
            AZ::TickBus::Handler::BusConnect();
        }
        else if (!shouldTick && m_isTickingPhysics)
        {
            AZ::TickBus::Handler::BusDisconnect();
        }
        m_isTickingPhysics = shouldTick;
    }

    void JoltPhysicsSystemComponent::ActivateSimulation()
    {
        m_joltSystem = GetJoltSystem();

        if (m_joltSystem)
        {
            // TODO: How to pass config in?
            // m_joltSystem->Initialize();
        }
    }
} // namespace JoltPhysics
