
#include <AzRefactor/JoltSystem.h>

#include <AzRefactor/JoltScene.h>

#include <AzCore/Asset/AssetManager.h>
#include <AzCore/Component/ComponentApplicationLifecycle.h>
#include <AzCore/Console/IConsole.h>
#include <AzCore/Debug/PerformanceCollector.h>
#include <AzCore/Debug/Profiler.h>
#include <AzCore/Math/MathUtils.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/PlatformId/PlatformId.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

// only enable physx timestep warning when not running debug or in Release
#if !defined(DEBUG) && !defined(RELEASE)
#define ENABLE_JOLT_TIMESTEP_WARNING
#endif

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(JoltSystem, AZ::SystemAllocator)

    // I don't know what the implementation of this looks like yet
#ifdef ENABLE_JOLT_TIMESTEP_WARNING
    namespace FrameTimeWarning
    {
        static constexpr int MaxSamples = 1000;
        static int NumSamples = 0;
        static int NumSamplesOverLimit = 0;
        static float LostTime = 0.0f;
    }

    AZ_CVAR(bool, jolt_reportTimestepWarnings, false, nullptr, AZ::ConsoleFunctorFlags::Null, "A flag providing ability to turn on/off reporting of Jolt timestep warnings");
#endif

    // A helper function.
    AZ::Debug::PerformanceCollector::DataLogType GetDataLogTypeFromCVar(const AZ::CVarFixedString& newCaptureType)
    {
        if (newCaptureType.starts_with('a') || newCaptureType.starts_with('A'))
        {
            return AZ::Debug::PerformanceCollector::DataLogType::LogAllSamples;
        }
        else
        {
            return AZ::Debug::PerformanceCollector::DataLogType::LogStatistics;
        }
    }

    JoltSystem::JoltSystem()
        // : m_sceneInterface(this)
    {

    }

    JoltSystem::~JoltSystem()
    {
        Shutdown();
    }

    void JoltSystem::Initialize([[maybe_unused]] const AzPhysics::SystemConfiguration* config)
    {
        if (m_state == State::Initialized)
        {
            AZ_Warning("JoltSystem", false, "Jolt system already initialized, Shutdown must be called first OR call Reinitialize or UpdateConfiguration(forceReinit=true) to reboot");
            return;
        }

        // TODO: Implement the AZ::SystemConfiguration and native Jolt variant
        // if (const auto* physXConfig = azdynamic_cast<const PhysXSystemConfiguration*>(config))
        // {
        //     m_systemConfig = *physXConfig;
        // }

        m_state = State::Initialized;
        // m_initializeEvent.Signal(&m_systemConfig);
    }

    void JoltSystem::Reinitialize()
    {
        AZ_Warning("JoltSystem", false, "Jolt Reinitialize currently not supported.");
    }

    void JoltSystem::Shutdown()
    {
        if (m_state != State::Initialized)
        {
            return;
        }

        RemoveAllScenes();

        m_accumulatedTime = 0.0f;
        m_state = State::Shutdown;
    }

    void JoltSystem::Simulate(float deltaTime)
    {
        AZ_PROFILE_FUNCTION(Physics);

        if (m_state != State::Initialized)
        {
            AZ_Warning("JoltSystem", false, "Called Simulate when Jolt system is not initialized");
            return;
        }

        auto simulateScenes = [this](float timeStep)
        {
            for (auto& scenePtr : m_sceneList)
            {
                if (scenePtr != nullptr && scenePtr->IsEnabled())
                {
                    AZ::Debug::ScopeDuration performanceScopeDuration(m_performanceCollector.get(), PerformanceSpecPhysXSimulationTime);
                    scenePtr->StartSimulation(timeStep);
                    scenePtr->FinishSimulation();
                }
            }
        };

        // deltaTime = AZ::GetClamp(deltaTime, 0.0f, m_systemConfig.m_maxTimestep);
        //
        // AZ_Assert(m_systemConfig.m_fixedTimestep >= 0.0f, "PhysXSystem - fixed timestep is negitive.");
        float tickTime = deltaTime;
        // if (m_systemConfig.m_fixedTimestep > 0.0f) //use the fixed timestep
        // {
        m_accumulatedTime += tickTime;
        //     //divide accumulated time by the fixed step and floor it to get the number of steps that would occur. Then multiply by fixedTimeStep to get the total executed time.
        //     tickTime = AZStd::floorf(m_accumulatedTime / m_systemConfig.m_fixedTimestep) * m_systemConfig.m_fixedTimestep;
        m_preSimulateEvent.Signal(tickTime);
        //
        //     while (m_accumulatedTime >= m_systemConfig.m_fixedTimestep)
        //     {
        //         simulateScenes(m_systemConfig.m_fixedTimestep);
        //         m_accumulatedTime -= m_systemConfig.m_fixedTimestep;
        //     }
        // }
        // else
        {
            m_preSimulateEvent.Signal(tickTime);

            simulateScenes(tickTime);
        }

        // m_postSimulateEvent.Signal(tickTime);
    }

    void JoltSystem::RemoveAllScenes()
    {
        m_sceneList.clear();

        //clear the free slots queue
        AZStd::queue<AzPhysics::SceneIndex> empty;
        m_freeSceneSlots.swap(empty);
    };


} // JoltPhysics
