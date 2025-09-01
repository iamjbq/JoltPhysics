
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

    void WorldSimulationOwner::Update(float inDeltaTime)
    {

        // TODO: figure out where system config lives
        auto updateSystems = [this](float timeStep)
        {
            for (auto& systemPtr : m_systemList)
            {
                if (systemPtr != nullptr)
                {
                    systemPtr->StartUpdate(timeStep);
                    systemPtr->FinishUpdate(); // Currently does nothing cause Jolt is pretty tidy already
                }
            }
        };

        inDeltaTime = AZ::GetClamp(inDeltaTime, 0.0f, 0.1f);
        float tickTime = inDeltaTime;

        if constexpr (tempFixedTimeStep > 0.0f) // use fixed time step
        {
            m_accumulatedTime += tickTime;
            //divide accumulated time by the fixed step and floor it to get the number of steps that would occur. Then multiply by fixedTimeStep to get the total executed time.
            tickTime = AZStd::floorf(m_accumulatedTime / tempFixedTimeStep) * tempFixedTimeStep;

            // m_preSimulateEvent.Signal(tickTime);

            while (m_accumulatedTime >= tempFixedTimeStep)
            {
                updateSystems(tempFixedTimeStep);
                m_accumulatedTime -= tempFixedTimeStep;
            }
        }
        else // otherwise, let's just get wild, shall we?
        {
            // m_preSimulateEvent.Signal(tickTime);
            updateSystems(tickTime);
        }
    }

    SystemHandle WorldSimulationOwner::AddPhysicsSystem(const SystemConfiguration& config)
    {
        SystemIndex freeIndex = m_freeSystemSlots.front();

        const SystemHandle systemHandle(AZ::Crc32(config.m_systemName), freeIndex);

        m_systemList[freeIndex] = AZStd::make_unique<JoltSystem>(config, systemHandle);

        // m_systemAddedEvent.Signal(systemHandle);

        return systemHandle;
    }

    WorldSimulationOwner* GetWorldSimulationOwner()
    {
        return azdynamic_cast<WorldSimulationOwner*>(AZ::Interface<JoltPhysics::WorldSimulationInterface>::Get());
    }
} // JoltPhysics
