
#include "JoltPhysicsSystemComponent.h"

#include <JoltPhysics/JoltPhysicsTypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

#include "System/WorldSimulationOwner.h"

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
        AZ::TickBus::Handler::BusConnect();
    }

    void JoltPhysicsSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        JoltPhysicsRequestBus::Handler::BusDisconnect();
    }

    void JoltPhysicsSystemComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_worldSimulationOwner)
        {
            m_worldSimulationOwner->Update(deltaTime);
        }
    }

    int JoltPhysicsSystemComponent::GetTickOrder()
    {
        return AZ::ComponentTickBus::TICK_PHYSICS_SYSTEM;
    }
} // namespace JoltPhysics
