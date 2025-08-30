
#include "WorldSimulationOwner.h"

namespace JoltPhysics
{
    WorldSimulationOwner::WorldSimulationOwner()
    {

    }

    WorldSimulationOwner::~WorldSimulationOwner()
    {

    }

    void WorldSimulationOwner::Initialize()
    {

    }

    SystemHandle WorldSimulationOwner::AddPhysicsSystem(const SystemConfiguration& config)
    {
        SystemIndex freeIndex = m_freeSystemSlots.front();

        const SystemHandle systemHandle(AZ::Crc32(config.m_systemName), freeIndex);

        m_systemList[freeIndex] = AZStd::make_unique<JoltPhysicsSystem>(config, systemHandle);

        // m_systemAddedEvent.Signal(systemHandle);

        return systemHandle;
    }
} // JoltPhysics
