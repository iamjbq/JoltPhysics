
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

    void WorldSimulationOwner::Update()
    {
        // TODO: figure out where system config lives and pass in config.m_timeStep
        auto updateSystems = [this](float timeStep)
        {
            for (auto& systemPtr : m_systemList)
            {
                if (systemPtr != nullptr)
                {
                    systemPtr->StartUpdate(timeStep);
                    systemPtr->FinishUpdate();
                }
            }
        };

        // TODO: need more here to track accumulated time, etc. See PhysXSystem.cpp Simulate() function
        updateSystems(0.0166667f);
    }

    SystemHandle WorldSimulationOwner::AddPhysicsSystem(const SystemConfiguration& config)
    {
        SystemIndex freeIndex = m_freeSystemSlots.front();

        const SystemHandle systemHandle(AZ::Crc32(config.m_systemName), freeIndex);

        m_systemList[freeIndex] = AZStd::make_unique<JoltSystem>(config, systemHandle);

        // m_systemAddedEvent.Signal(systemHandle);

        return systemHandle;
    }
} // JoltPhysics
