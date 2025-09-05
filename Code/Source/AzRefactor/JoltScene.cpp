
#include <AzRefactor/JoltScene.h>

#include <AzCore/Console/IConsole.h>
#include <AzCore/Debug/ProfilerBus.h>
#include <AzCore/std/algorithm.h>
#include <AzCore/std/containers/variant.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/Debug/Profiler.h>
#include <AzCore/Task/TaskGraph.h>
#include <AzFramework/Physics/Character.h>
#include <AzFramework/Physics/Collision/CollisionEvents.h>
#include <AzFramework/Physics/Configuration/RigidBodyConfiguration.h>
#include <AzFramework/Physics/Configuration/StaticRigidBodyConfiguration.h>
#include <AzFramework/Physics/Material/PhysicsMaterialManager.h>
#include <AzFramework/Physics/Configuration/SystemConfiguration.h>

#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(JoltScene, AZ::SystemAllocator, 0)

    JoltScene::JoltScene(const AzPhysics::SceneConfiguration& config, const AzPhysics::SceneHandle& sceneHandle)
        : Scene(config)
        , m_config(config)
        , m_sceneHandle(sceneHandle)
        , m_physicsSystemConfigChanged([this](const AzPhysics::SystemConfiguration* config)
        {
            m_raycastBufferSize = config->m_raycastBufferSize;
            m_shapecastBufferSize = config->m_shapecastBufferSize;
            m_overlapBufferSize = config->m_overlapBufferSize;
        })
    {
        m_joltSystem = AZStd::make_unique<JPH::PhysicsSystem>();

        AZ_Assert(m_joltSystem != nullptr, "JPH::PhysicsSystem creation failed.");

        m_gravity = m_config.m_gravity;
    }

    JoltScene::~JoltScene()
    {
        for (auto& simulatedBody : m_simulatedBodies)
        {
            if (simulatedBody.second != nullptr)
            {
                if (simulatedBody.second->m_simulating)
                {
                    // Disable simulation on body (not signaling OnSimulationBodySimulationDisabled event)
                    // DisableSimulationOfBodyInternal(*simulatedBody.second);
                }
                m_simulatedBodyRemovedEvent.Signal(m_sceneHandle, simulatedBody.second->m_bodyHandle);
                delete simulatedBody.second;
            }
        }
        m_simulatedBodies.clear();

        if (m_joltSystem)
        {
            m_joltSystem = nullptr;
        }
    }

    void JoltScene::StartSimulation(float deltatime)
    {
        AZ_PROFILE_SCOPE(Physics, "JoltScene::StartSimulation");

        if (!IsEnabled())
        {
            return;
        }

        {
            AZ_PROFILE_SCOPE(Physics, "OnSceneSimulationStartEvent::Signaled");
            m_sceneSimulationStartEvent.Signal(m_sceneHandle, deltatime);
        }

        m_currentDeltaTime = deltatime;

        m_joltSystem->Update(deltatime, m_collisionSteps, m_tempAllocator, m_jobSystem);
    }

    void JoltScene::FinishSimulation()
    {
        AZ_PROFILE_SCOPE(Physics, "JoltScene::FinishSimulation");

        if (!IsEnabled())
        {
            return;
        }
    }

    void JoltScene::SetEnabled(bool enable)
    {
        m_isEnabled = enable;
    }

    bool JoltScene::IsEnabled() const
    {
        return m_isEnabled;
    }

    const AzPhysics::SceneConfiguration& JoltScene::GetConfiguration() const
    {
        return m_config;
    }

    void JoltScene::UpdateConfiguration(const AzPhysics::SceneConfiguration& config)
    {
        if (m_config != config)
        {
            m_config = config;
            m_configChangeEvent.Signal(m_sceneHandle, m_config);

            // set gravity verifies this is a new value.
            SetGravity(m_config.m_gravity);
        }
    };


}
