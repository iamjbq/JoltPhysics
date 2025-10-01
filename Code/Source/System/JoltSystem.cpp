

#include <AzCore/Asset/AssetManager.h>
#include <AzCore/Component/ComponentApplicationLifecycle.h>
#include <AzCore/Console/IConsole.h>
#include <AzCore/Debug/PerformanceCollector.h>
#include <AzCore/Debug/Profiler.h>
#include <AzCore/Math/MathUtils.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/PlatformId/PlatformId.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzFramework/Physics/Configuration/CollisionConfiguration.h>

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>
// #include <Jolt/Core/JobSystemThreadPool.h>

#include <System/JoltSystem.h>
#include <Scene/JoltScene.h>
#include <System/JoltJobSystemThreaded.h>
#include <JoltPhysics/Configuration/JoltConfiguration.h>

// only enable Jolt timestep warning when not running debug or in Release
#if !defined(DEBUG) && !defined(RELEASE)
#define ENABLE_JOLT_TIMESTEP_WARNING
#endif

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(JoltSystem, AZ::SystemAllocator)

    // Disable common warnings triggered by Jolt, you can use JPH_SUPPRESS_WARNING_PUSH / JPH_SUPPRESS_WARNING_POP to store and restore the warning state
    JPH_SUPPRESS_WARNINGS

    // If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
    using namespace JPH::literals;
    
    // Callback for traces, connect this to your own trace function if you have one
    static void JoltTraceImpl(const char *inFMT, ...)
    {
        AZ_Trace("Jolt", inFMT)
    }
    
#ifdef JPH_ENABLE_ASSERTS
    
    // Callback for asserts, connect this to your own assert handler if you have one
    static bool JoltAssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, JPH::uint inLine)
    {
        AZ_Assert(false, "Jolt - %s:%i: (%s) %s", inFile, inLine, inExpression, (inMessage != nullptr? inMessage : ""))

        // Breakpoint
        return true;
    };
    
#endif // JPH_ENABLE_ASSERTS

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
        : m_sceneInterface(this)
    {
    }

    JoltSystem::~JoltSystem()
    {
        Shutdown();
    }

    void JoltSystem::Initialize(const AzPhysics::SystemConfiguration* config)
    {
        if (m_state == State::Initialized)
        {
            AZ_Warning("JoltSystem", false, "Jolt system already initialized, Shutdown must be called first OR call Reinitialize or UpdateConfiguration(forceReinit=true) to reboot");
            return;
        }

        if (const auto* joltConfig = azdynamic_cast<const JoltSystemConfiguration*>(config))
        {
            m_systemConfig = *joltConfig;
        }

        const AZStd::vector<AzPhysics::CollisionGroups::Preset>& presets = m_systemConfig.m_collisionConfig.m_collisionGroups.GetPresets();

        // We need to store the raw bitmasks for direct collision lookup in Jolt
        // Since we don't have anything convenient like pxFilterData
        for (const auto& preset : presets)
        {
            m_collisionGroupMasks.push_back(preset.m_group.GetMask());
        }

        JPH::RegisterDefaultAllocator();
        
        JPH::Trace = JoltTraceImpl;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = JoltPhysics::JoltAssertFailedImpl);
        
        // TODO: Eventually create a JoltAllocator class in O3DE
        // JPH::Allocate = ;
        // JPH::Free = ;
        // JPH::Reallocate = ;
        // JPH::AlignedAllocate = ;
        // JPH::AlignedFree = ;

        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        m_allocator = AZStd::make_unique<JPH::TempAllocatorImpl>(cAllocationArenaSize);
        m_jobSystem = AZStd::make_unique<JoltJobSystemThreaded>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, AZStd::thread::hardware_concurrency() - 1);
        // m_tempJobSystem = AZStd::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, AZStd::thread::hardware_concurrency() - 1);

        m_state = State::Initialized;
        m_initializeEvent.Signal(&m_systemConfig);
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

        // Unregisters all types with the factory and cleans up the default material
        JPH::UnregisterTypes();

        // Destroy the factory
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;

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
                    AZ::Debug::ScopeDuration performanceScopeDuration(m_performanceCollector.get(), PerformanceSpecJoltSimulationTime);
                    scenePtr->StartSimulation(timeStep);
                    scenePtr->FinishSimulation();
                }
            }
        };

        deltaTime = AZ::GetClamp(deltaTime, 0.0f, m_systemConfig.m_maxTimestep);

        AZ_Assert(m_systemConfig.m_fixedTimestep >= 0.0f, "JoltSystem - fixed timestep is negative.");
        float tickTime = deltaTime;
        if (m_systemConfig.m_fixedTimestep > 0.0f) //use the fixed timestep
        {
            m_accumulatedTime += tickTime;

            // Divide accumulated time by the fixed step and floor it to get the number of steps that would occur.
            // Then multiply by fixedTimeStep to get the total executed time.
            tickTime = AZStd::floorf(m_accumulatedTime / m_systemConfig.m_fixedTimestep) * m_systemConfig.m_fixedTimestep;
            m_preSimulateEvent.Signal(tickTime);

            while (m_accumulatedTime >= m_systemConfig.m_fixedTimestep)
            {
                simulateScenes(m_systemConfig.m_fixedTimestep);
                m_accumulatedTime -= m_systemConfig.m_fixedTimestep;
            }
        }
        else
        {
            m_preSimulateEvent.Signal(tickTime);

            simulateScenes(tickTime);
        }

        m_postSimulateEvent.Signal(tickTime);
    }

    AzPhysics::SceneHandle JoltSystem::AddScene(const AzPhysics::SceneConfiguration& config)
    {
        if (config.m_sceneName.empty())
        {
            AZ_Error("JoltSystem", false, "AddScene: Trying to Add a scene without a name. SceneConfiguration::m_sceneName must have a value");
            return AzPhysics::InvalidSceneHandle;
        }

        if (!m_freeSceneSlots.empty()) //fill any free slots first before increasing the size of the scene list vector.
        {
            AzPhysics::SceneIndex freeIndex = m_freeSceneSlots.front();
            m_freeSceneSlots.pop();

            AZ_Assert(freeIndex < m_sceneList.size(), "JoltSystem::AddScene: Free scene index is out of bounds!");
            AZ_Assert(m_sceneList[freeIndex] == nullptr, "JoltSystem::AddScene: Free scene index is not free");

            const AzPhysics::SceneHandle sceneHandle(AZ::Crc32(config.m_sceneName), freeIndex);
            m_sceneList[freeIndex] = AZStd::make_unique<JoltScene>(config, sceneHandle);
            m_sceneAddedEvent.Signal(sceneHandle);
            return sceneHandle;
        }

        if (m_sceneList.size() < AzPhysics::MaxNumberOfScenes) //add a new scene if it is under the limit
        {
            const AzPhysics::SceneHandle sceneHandle(AZ::Crc32(config.m_sceneName), static_cast<AzPhysics::SceneIndex>(m_sceneList.size()));
            m_sceneList.emplace_back(AZStd::make_unique<JoltScene>(config, sceneHandle));
            m_sceneAddedEvent.Signal(sceneHandle);
            return sceneHandle;
        }
        AZ_Warning("Jolt", false, "Scene Limit reached[%u], unable to add new scene [%s]",
            AzPhysics::MaxNumberOfScenes,
            config.m_sceneName.c_str());
        return AzPhysics::InvalidSceneHandle;
    }

    AzPhysics::SceneHandleList JoltSystem::AddScenes(const AzPhysics::SceneConfigurationList& configs)
    {
        AzPhysics::SceneHandleList sceneHandles;
        sceneHandles.reserve(configs.size());
        for (const auto& config : configs)
        {
            AzPhysics::SceneHandle sceneHandle = AddScene(config);
            sceneHandles.emplace_back(sceneHandle);
        }
        return sceneHandles;
    }

    AzPhysics::SceneHandle JoltSystem::GetSceneHandle(const AZStd::string& sceneName)
    {
        const AZ::Crc32 sceneCrc(sceneName);
        auto sceneItr = AZStd::find_if(m_sceneList.begin(), m_sceneList.end(), [sceneCrc](auto& scene) {
            return scene != nullptr && sceneCrc == scene->GetId();
            });

        if (sceneItr != m_sceneList.end())
        {
            return AzPhysics::SceneHandle((*sceneItr)->GetId(), static_cast<AzPhysics::SceneIndex>(AZStd::distance(m_sceneList.begin(), sceneItr)));
        }
        return AzPhysics::InvalidSceneHandle;
    }

    AzPhysics::Scene* JoltSystem::GetScene(AzPhysics::SceneHandle handle)
    {
        if (handle == AzPhysics::InvalidSceneHandle)
        {
            return nullptr;
        }

        AzPhysics::SceneIndex index = AZStd::get<AzPhysics::HandleTypeIndex::Index>(handle);
        if (index < m_sceneList.size())
        {
            if (auto& scenePtr = m_sceneList[index];
                scenePtr != nullptr)
            {
                if (scenePtr->GetId() == AZStd::get<AzPhysics::HandleTypeIndex::Crc>(handle))
                {
                    return scenePtr.get();
                }
            }
        }
        return nullptr;
    }

    AzPhysics::SceneList JoltSystem::GetScenes(const AzPhysics::SceneHandleList& handles)
    {
        AzPhysics::SceneList requestedSceneList;
        requestedSceneList.reserve(handles.size());
        for (const auto& handle : handles)
        {
            AzPhysics::Scene* scene = GetScene(handle);
            requestedSceneList.emplace_back(scene);
        }
        return requestedSceneList;
    }

    AzPhysics::SceneList& JoltSystem::GetAllScenes()
    {
        return m_sceneList;
    }

    void JoltSystem::RemoveScene(AzPhysics::SceneHandle handle)
    {
        if (handle == AzPhysics::InvalidSceneHandle)
        {
            return;
        }

        AZ::u64 index = AZStd::get<AzPhysics::HandleTypeIndex::Index>(handle);
        if (index < m_sceneList.size() )
        {
            if (auto& scenePtr = m_sceneList[index];
                scenePtr != nullptr)
            {
                if (scenePtr->GetId() == AZStd::get<AzPhysics::HandleTypeIndex::Crc>(handle))
                {
                    m_sceneRemovedEvent.Signal(handle);
                    m_sceneList[index].reset();
                    m_freeSceneSlots.push(static_cast<AzPhysics::SceneIndex>(index));
                }
            }
        }
    }

    void JoltSystem::RemoveScenes(const AzPhysics::SceneHandleList& handles)
    {
        for (const auto& handle : handles)
        {
            RemoveScene(handle);
        }
    }

    void JoltSystem::RemoveAllScenes()
    {
        m_sceneList.clear();

        //clear the free slots queue
        AZStd::queue<AzPhysics::SceneIndex> empty;
        m_freeSceneSlots.swap(empty);
    }

    AZStd::pair<AzPhysics::SceneHandle, AzPhysics::SimulatedBodyHandle> JoltSystem::FindAttachedBodyHandleFromEntityId(
        AZ::EntityId entityId)
    {
        for (auto& scenePtr : m_sceneList)
        {
            if (scenePtr == nullptr)
            {
                continue;
            }
            if (auto* joltScene = azdynamic_cast<JoltScene*>(scenePtr.get()))
            {
                for (const auto& [_, body] : joltScene->GetSimulatedBodyList())
                {
                    if (body != nullptr && body->GetEntityId() == entityId)
                    {
                        return AZStd::make_pair(joltScene->GetSceneHandle(), body->m_bodyHandle);
                    }
                }
            }
        }
        return AZStd::make_pair(AzPhysics::InvalidSceneHandle, AzPhysics::InvalidSimulatedBodyHandle);
    }

    const AzPhysics::SystemConfiguration* JoltSystem::GetConfiguration() const
    {
        return &m_systemConfig;
    }

    void JoltSystem::UpdateConfiguration(const AzPhysics::SystemConfiguration* newConfig, [[maybe_unused]] bool forceReinitialization)
    {
        if (const auto* joltConfig = azdynamic_cast<const JoltSystemConfiguration*>(newConfig);
            m_systemConfig != (*joltConfig))
        {
            m_systemConfig = (*joltConfig);
            m_configChangeEvent.Signal(joltConfig);

            // May need to restart simulation, as per PhysX comment
        }
    }

    void JoltSystem::UpdateDefaultSceneConfiguration(const AzPhysics::SceneConfiguration& sceneConfiguration)
    {
        if (m_defaultSceneConfiguration != sceneConfiguration)
        {
            m_defaultSceneConfiguration = sceneConfiguration;

            m_onDefaultSceneConfigurationChangedEvent.Signal(&m_defaultSceneConfiguration);
        }
    }

    const AzPhysics::SceneConfiguration& JoltSystem::GetDefaultSceneConfiguration() const
    {
        return m_defaultSceneConfiguration;
    }

    // These are noted in PhysX to be temporary, but unsure if this is still true.

    void JoltSystem::SetCollisionLayerName(int index, const AZStd::string& layerName)
    {
        m_systemConfig.m_collisionConfig.m_collisionLayers.SetName(aznumeric_cast<AZ::u64>(index), layerName);
    }

    void JoltSystem::CreateCollisionGroup(const AZStd::string& groupName, const AzPhysics::CollisionGroup& group)
    {
        m_systemConfig.m_collisionConfig.m_collisionGroups.CreateGroup(groupName, group);
    }

    AZ::Debug::PerformanceCollector* JoltSystem::GetPerformanceCollector()
    {
        return m_performanceCollector.get();
    }

    AZ::u32 JoltSystem::GetCollisionGroupIndex(const AzPhysics::CollisionGroup& group) const
    {
        const AZStd::vector<AzPhysics::CollisionGroups::Preset>& presets = m_systemConfig.m_collisionConfig.m_collisionGroups.GetPresets();

        for (AZ::u32 i = 0; i < presets.size(); ++i)
        {
            if (presets.at(i).m_group == group)
                return i;
        }
        AZ_Warning("JoltSystem", false, "Unable to find a match for group in vector of Presets.")
        return 0;
    }

    AZ::u32 JoltSystem::GetCollisionGroupIndex(const AzPhysics::CollisionGroups::Id& groupId) const
    {
        const AZStd::vector<AzPhysics::CollisionGroups::Preset>& presets = m_systemConfig.m_collisionConfig.m_collisionGroups.GetPresets();

        for (AZ::u32 i = 0; i < presets.size(); ++i)
        {
            if (presets.at(i).m_id == groupId)
                return i;
        }
        AZ_Warning("JoltSystem", false, "Unable to find a match for group Id in vector of Presets.")
        return 0;
    }

    AZ::u64 JoltSystem::GetCollisionMask(AZ::u32 index) const
    {
        return m_collisionGroupMasks.at(index);
    }

    AZStd::fixed_vector<AZ::u64, AzPhysics::CollisionLayers::MaxCollisionLayers>* JoltSystem::GetCollisionMasks()
    {
        return &m_collisionGroupMasks;
    }

    JPH::TempAllocatorImpl* JoltSystem::GetJoltAllocator()
    {
        return m_allocator.get();
    }

    JoltJobSystemThreaded* JoltSystem::GetJoltJobSystem()
    {
        return m_jobSystem.get();
    }

    // JPH::JobSystemThreadPool* JoltSystem::GetTempJobSystem()
    // {
    //     return m_tempJobSystem.get();
    // }

    BroadPhaseLayerInterfaceImpl& JoltSystem::GetBroadPhaseLayerInterface()
    {
        return m_broadPhaseInterface;
    }

    ObjectVsBroadPhaseLayerFilterImpl& JoltSystem::GetObjectVsBroadPhaseLayerFilter()
    {
        return m_objectVsBroadPhaseLayerFilter;
    }

    ObjectLayerPairFilterImpl& JoltSystem::GetObjectLayerPairFilter()
    {
        return m_objectLayerPairFilter;
    }

    JoltSystem* GetJoltSystem()
    {
        return azdynamic_cast<JoltSystem*>(AZ::Interface<AzPhysics::SystemInterface>::Get());
    }

} // JoltPhysics
