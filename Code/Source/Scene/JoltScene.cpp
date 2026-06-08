
#include "JoltScene.h"
#include <System/JoltSystem.h>
#include <Clients/RigidBody.h>
#include <Clients/StaticRigidBody.h>
#include <Clients/Shape.h>
#include <JoltPhysics/Utils.h>
#include <JoltPhysics/MathConversions.h>
#include <System/CollisionLayerFilters.h>
#include <System/JoltJobSystemThreaded.h>
#include <Scene/PhysicsSystemCallbacks.h>

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

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/JobSystemThreadPool.h>

namespace JoltPhysics
{
    AZ_CVAR_EXTERNED(bool, jolt_batchTransformSync);

    AZ_CVAR(bool, jolt_parallelTransformSync, true, nullptr, AZ::ConsoleFunctorFlags::Null, "Multithreaded transform update for rigid bodies. "
        "Only relevant if batched transform update is enabled.");
    AZ_CVAR(size_t, jolt_parallelTransformSyncBatchSize, 250, nullptr, AZ::ConsoleFunctorFlags::Null,
        "How many rigid bodies should be processed per task");

    namespace Internal
    {
        bool AddShape(AZStd::variant<AzPhysics::RigidBody*, AzPhysics::StaticRigidBody*> simulatedBody, const AzPhysics::ShapeVariantData& shapeData)
        {
            if (const auto* shapeColliderPair = AZStd::get_if<AzPhysics::ShapeColliderPair>(&shapeData))
            {
                bool shapeAdded = false;
                auto shapePtr = AZStd::make_shared<Shape>(*(shapeColliderPair->first), *(shapeColliderPair->second));
                AZStd::visit([shapePtr, &shapeAdded](auto&& body)
                    {
                        if (shapePtr->GetNativePointer())
                        {
                            body->AddShape(shapePtr);
                            shapeAdded = true;
                        }
                    }, simulatedBody);
                return shapeAdded;
            }
            if (const auto* shapeColliderPairList = AZStd::get_if<AZStd::vector<AzPhysics::ShapeColliderPair>>(&shapeData))
            {
                bool shapeAdded = false;
                for (const auto& shapeColliderConfigs : *shapeColliderPairList)
                {
                    auto shapePtr = AZStd::make_shared<Shape>(*(shapeColliderConfigs.first), *(shapeColliderConfigs.second));
                    AZStd::visit([shapePtr, &shapeAdded](auto&& body)
                    {
                        if (shapePtr->GetNativePointer())
                        {
                            body->AddShape(shapePtr);
                            shapeAdded = true;
                        }
                    }, simulatedBody);
                }
                return shapeAdded;
            }
            if (const auto* shape = AZStd::get_if<AZStd::shared_ptr<Physics::Shape>>(&shapeData))
            {
                auto shapePtr = *shape;
                AZStd::visit([shapePtr](auto&& body)
                {
                    body->AddShape(shapePtr);
                }, simulatedBody);
                return true;
            }
            else if (const auto* shapeList = AZStd::get_if<AZStd::vector<AZStd::shared_ptr<Physics::Shape>>>(&shapeData))
            {
                for (auto shapePtr : *shapeList)
                {
                    AZStd::visit([shapePtr](auto&& body)
                    {
                        body->AddShape(shapePtr);
                    }, simulatedBody);
                }
                return true;
            }
            return false;
        }

        template<class SimulatedBodyType, class ConfigurationType>
        AzPhysics::SimulatedBody* CreateSimulatedBody(const ConfigurationType* configuration, AZ::Crc32& crc, JPH::PhysicsSystem& inSystem)
        {
            auto* newBody = aznew SimulatedBodyType(*configuration, inSystem);
            if (!AZStd::holds_alternative<AZStd::monostate>(configuration->m_colliderAndShapeData))
            {
                const bool shapeAdded = AddShape(newBody, configuration->m_colliderAndShapeData);
                AZ_Warning("JoltScene::CreateSimulatedBody", shapeAdded, "No Collider or Shape information found when creating Rigid body [%s]", configuration->m_debugName.c_str());
                newBody->BuildCompoundShape();
            }
            crc = AZ::Crc32(newBody, sizeof(*newBody));
            return newBody;
        }

        AzPhysics::SimulatedBody* CreateRigidBody(const AzPhysics::RigidBodyConfiguration* configuration, AZ::Crc32& crc, JPH::PhysicsSystem& inSystem)
        {
            auto* newBody = aznew RigidBody(*configuration, inSystem);
            if (!AZStd::holds_alternative<AZStd::monostate>(configuration->m_colliderAndShapeData))
            {
                const bool shapeAdded = AddShape(newBody, configuration->m_colliderAndShapeData);
                AZ_Warning("JoltScene", shapeAdded, "No Collider or Shape information found when creating Rigid body [%s]", configuration->m_debugName.c_str());
                newBody->BuildFinalShape();
            }
            const AzPhysics::MassComputeFlags& flags = configuration->GetMassComputeFlags();
            newBody->UpdateMassProperties(flags, configuration->m_centerOfMassOffset,
                configuration->m_inertiaTensor, configuration->m_mass);

            crc = AZ::Crc32(newBody, sizeof(*newBody));
            return newBody;
        }

    }

    AZ_CLASS_ALLOCATOR_IMPL(JoltScene, AZ::SystemAllocator, 0)

    JoltScene::JoltScene(const AzPhysics::SceneConfiguration& config, const AzPhysics::SceneHandle& sceneHandle)
        : Scene(config)
        , m_config(config)
        , m_sceneHandle(sceneHandle)
        , m_physicsSystemConfigChanged([this](const AzPhysics::SystemConfiguration* sysConfig)
        {
            const auto* joltConfig = azdynamic_cast<const JoltSystemConfiguration*>(sysConfig);
            m_cachedSystemConfig = AZStd::move(*joltConfig);

            m_maxBodies = joltConfig->m_systemInitSettings.m_maxBodies;
            m_numBodyMutexes = joltConfig->m_systemInitSettings.m_numBodyMutexes;
            m_maxBodyPairs = joltConfig->m_systemInitSettings.m_maxBodyPairs;
            m_maxContactConstraints = joltConfig->m_systemInitSettings.m_maxContactConstraints;
            m_collisionSteps = joltConfig->m_systemInitSettings.m_collisionSteps;

            m_raycastBufferSize = joltConfig->m_raycastBufferSize;
            m_shapecastBufferSize = joltConfig->m_shapecastBufferSize;
            m_overlapBufferSize = joltConfig->m_overlapBufferSize;
        })
    {

        // Setup Jolt physics system variables
        if (JoltSystem* system = GetJoltSystem())
        {
            const JoltSystemConfiguration& joltConfig = system->GetJoltConfiguration();
            m_cachedSystemConfig = AZStd::move(joltConfig);

            m_maxBodies = joltConfig.m_systemInitSettings.m_maxBodies;
            m_numBodyMutexes = joltConfig.m_systemInitSettings.m_numBodyMutexes;
            m_maxBodyPairs = joltConfig.m_systemInitSettings.m_maxBodyPairs;
            m_maxContactConstraints = joltConfig.m_systemInitSettings.m_maxContactConstraints;
            m_collisionSteps = joltConfig.m_systemInitSettings.m_collisionSteps; // caching this as it's used every update

            m_raycastBufferSize = joltConfig.m_raycastBufferSize;
            m_shapecastBufferSize = joltConfig.m_shapecastBufferSize;
            m_overlapBufferSize = joltConfig.m_overlapBufferSize;

            m_jobSystem = system->GetJoltJobSystem();
            m_tempAllocator = system->GetJoltAllocator();

            system->RegisterSystemConfigurationChangedEvent(m_physicsSystemConfigChanged);
        }

        m_physicsSystem = AZStd::make_unique<JPH::PhysicsSystem>();

        AZ_Assert(m_physicsSystem != nullptr, "JPH::PhysicsSystem creation failed.");

        m_gravity = m_config.m_gravity;

        InitializeJoltSystem();
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
                    DisableSimulationOfBodyInternal(*simulatedBody.second);
                }
                m_simulatedBodyRemovedEvent.Signal(m_sceneHandle, simulatedBody.second->m_bodyHandle);
                delete simulatedBody.second;
            }
        }
        m_simulatedBodies.clear();

        if (m_physicsSystem)
        {
            m_physicsSystem = nullptr;
        }
    }

    void JoltScene::StartSimulation(float deltaTime)
    {
        AZ_PROFILE_SCOPE(Physics, "JoltScene::StartSimulation");

        if (!IsEnabled())
        {
            return;
        }

        {
            AZ_PROFILE_SCOPE(Physics, "OnSceneSimulationStartEvent::Signaled");
            m_sceneSimulationStartEvent.Signal(m_sceneHandle, deltaTime);
        }
        
        if (!m_bodiesToAdd.empty())
        {
            AZ_Assert(sizeof(JPH::BodyID) == sizeof(AZ::u32), "StartSimulation: Jolt BodyId was incorrect size")
            
            JPH::BodyID* bodyIDs = reinterpret_cast<JPH::BodyID*>(m_bodiesToAdd.data());
            const auto count = static_cast<int>(m_bodiesToAdd.size());
            void* bodiesData = m_bodyInterface->AddBodiesPrepare(bodyIDs, count);
            m_bodyInterface->AddBodiesFinalize(bodyIDs, count, bodiesData, JPH::EActivation::DontActivate);
            
            // In case we need to handle BodyID sequence rollover after 128 re-uses in a very short window, we can use this approach
            // AZ::u32 startIndex = 0;
            // while (startIndex < m_bodiesToAdd.size())
            // {
            //     const AZ::u32 count = AZStd::min(static_cast<AZ::u32>(m_bodiesToAdd.size()) - startIndex, MaxBodiesPerBatchAdd);
            //     
            //     JPH::BodyID* bodyIDs = reinterpret_cast<JPH::BodyID*>(m_bodiesToAdd.data() + startIndex);
            //     
            //     void* bodiesData = m_bodyInterface->AddBodiesPrepare(bodyIDs, count);
            //     m_bodyInterface->AddBodiesFinalize(bodyIDs, count, bodiesData, JPH::EActivation::DontActivate);
            //    
            //     
            //     startIndex += count;
            // }
            
            m_bodiesToAdd.clear();
        }
        
        if (!m_bodiesToAddAndActivate.empty())
        {
            AZ_Assert(sizeof(JPH::BodyID) == sizeof(AZ::u32), "StartSimulation: Jolt BodyId was incorrect size")
            
            JPH::BodyID* bodyIDs = reinterpret_cast<JPH::BodyID*>(m_bodiesToAddAndActivate.data());
            const auto count = static_cast<int>(m_bodiesToAddAndActivate.size());
            void* bodiesData = m_bodyInterface->AddBodiesPrepare(bodyIDs, count);
            m_bodyInterface->AddBodiesFinalize(bodyIDs, count, bodiesData, JPH::EActivation::Activate);
            
            
            // TODO: in case we need this to be threadsafe in the future, something like this might be necessary during body queuing 
            // {
            //     AZStd::scoped_lock lock(m_bodyQueueMutex); // acquires mutex here
            //     m_bodiesToAdd.push_back(bodyID);
            // } // lock destructor releases mutex here automatically
            
            m_bodiesToAddAndActivate.clear();
        }

        m_currentDeltaTime = deltaTime;
        m_physicsSystem->Update(deltaTime, m_collisionSteps, m_tempAllocator, m_jobSystem);
    }

    void JoltScene::FinishSimulation()
    {
        AZ_PROFILE_SCOPE(Physics, "JoltScene::FinishSimulation");

        if (!IsEnabled())
        {
            return;
        }

        bool activeBodiesEnabled = false;
        {
            AZ_PROFILE_SCOPE(Physics, "JoltScene::GetNumActiveBodies");
            activeBodiesEnabled = m_physicsSystem->GetNumActiveBodies(JPH::EBodyType::RigidBody) > 0
            // || m_physicsSystem->GetNumActiveBodies(JPH::EBodyType::SoftBody) > 0
            ;
        }

        if (activeBodiesEnabled)
        {
            AZ_PROFILE_SCOPE(Physics, "JoltScene::ProcessActiveBodies");

            AzPhysics::SimulatedBodyHandleList activeBodyHandles;

            {
                AZ::u32 activeRigidBodiesCount = 0;
                activeRigidBodiesCount = m_physicsSystem->GetNumActiveBodies(JPH::EBodyType::RigidBody);

                activeBodyHandles.reserve(activeRigidBodiesCount);
                JPH::BodyIDVector activeRigidBodies;
                // JPH::BodyIDVector activeSoftBodies;
                m_physicsSystem->GetActiveBodies(JPH::EBodyType::RigidBody,activeRigidBodies);
                // m_physicsSystem->GetActiveBodies(JPH::EBodyType::SoftBody,activeSoftBodies);

                for (AZ::u32 i = 0; i < activeRigidBodiesCount; ++i)
                {
                    if (auto* bodyData = reinterpret_cast<BodyData*>(m_bodyInterface->GetUserData(activeRigidBodies[i])))
                    {
                        activeBodyHandles.emplace_back(bodyData->GetBodyHandle());
                    }
                }
            }

            // Keep the event signal outside the scene lock since there may be handlers that want to lock the scene for write
            m_sceneActiveSimulatedBodies.Signal(m_sceneHandle, activeBodyHandles, m_currentDeltaTime);

            SyncActiveBodyTransform(activeBodyHandles);
        }

        FlushQueuedEvents();
        ClearDeferredDeletions();

        {
            AZ_PROFILE_SCOPE(Physics, "OnSceneSimulationFinishedEvent::Signaled");
            m_sceneSimulationFinishEvent.Signal(m_sceneHandle, m_currentDeltaTime);
        }

        // UpdateAzProfilerDataPoints();
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
    }

    AzPhysics::SimulatedBodyHandle JoltScene::AddSimulatedBody(const AzPhysics::SimulatedBodyConfiguration* simulatedBodyConfig)
    {
        // TODO: Missing character config, ragdoll, articulation

        AzPhysics::SimulatedBody* newBody = nullptr;
        AZ::Crc32 newBodyCrc;
        if (azrtti_istypeof<AzPhysics::RigidBodyConfiguration>(simulatedBodyConfig))
        {
            newBody = Internal::CreateRigidBody(
                azdynamic_cast<const AzPhysics::RigidBodyConfiguration*>(simulatedBodyConfig), newBodyCrc, *m_physicsSystem);
        }
        else if (azrtti_istypeof<AzPhysics::StaticRigidBodyConfiguration>(simulatedBodyConfig))
        {
            newBody = Internal::CreateSimulatedBody<StaticRigidBody, AzPhysics::StaticRigidBodyConfiguration>(
                azdynamic_cast<const AzPhysics::StaticRigidBodyConfiguration*>(simulatedBodyConfig), newBodyCrc, *m_physicsSystem);
        }
        // else if (azrtti_istypeof<Physics::CharacterConfiguration>(simulatedBodyConfig))
        // {
        //     newBody = Internal::CreateCharacterBody(this, azdynamic_cast<const Physics::CharacterConfiguration*>(simulatedBodyConfig));
        // }
        // else if (azrtti_istypeof<Physics::RagdollConfiguration>(simulatedBodyConfig))
        // {
        //     newBody = Internal::CreateRagdollBody(this, azdynamic_cast<const Physics::RagdollConfiguration*>(simulatedBodyConfig));
        // }
        // else if (azrtti_istypeof<ArticulationLinkConfiguration>(simulatedBodyConfig))
        // {
        //     newBody = JoltPhysics::CreateArticulationLink(azdynamic_cast<const ArticulationLinkConfiguration*>(simulatedBodyConfig));
        // }
        else
        {
            AZ_Warning("JoltScene", false, "Unknown SimulatedBodyConfiguration.");
            return AzPhysics::InvalidSimulatedBodyHandle;
        }

        if (newBody != nullptr)
        {
            AzPhysics::SimulatedBodyIndex index;

            if (m_freeSceneSlots.empty())
            {
                m_simulatedBodies.emplace_back(newBodyCrc, newBody);
                index = static_cast<AzPhysics::SimulatedBodyIndex>(m_simulatedBodies.size() - 1);
            }
            else
            {
                // Fill any free slots first before increasing the size of the simulatedBodies vector.
                index = m_freeSceneSlots.front();
                m_freeSceneSlots.pop();
                AZ_Assert(index < m_simulatedBodies.size(), "JoltScene::AddSimulatedBody: Free simulated body index is out of bounds");
                AZ_Assert(m_simulatedBodies[index].second == nullptr, "JoltScene::AddSimulatedBody: Free simulated body index is not free");

                m_simulatedBodies[index] = AZStd::make_pair(newBodyCrc, newBody);
            }

            const AzPhysics::SimulatedBodyHandle newBodyHandle(newBodyCrc, index);
            newBody->m_sceneOwner = m_sceneHandle;
            newBody->m_bodyHandle = newBodyHandle;
            m_simulatedBodyAddedEvent.Signal(m_sceneHandle, newBodyHandle);
            
            // // Enable simulation by default (not signaling OnSimulationBodySimulationEnabled event)
            // if (simulatedBodyConfig->m_startSimulationEnabled)
            // {
            //     EnableSimulationOfBodyInternal(*newBody); // TODO: Editor RBs should always start not Activated (asleep)
            // }

            return newBodyHandle;
        }

        return AzPhysics::InvalidSimulatedBodyHandle;
    }

    AzPhysics::SimulatedBodyHandleList JoltScene::AddSimulatedBodies(const AzPhysics::SimulatedBodyConfigurationList& simulatedBodyConfigs)
    {
        AzPhysics::SimulatedBodyHandleList newBodyHandles;
        newBodyHandles.reserve(simulatedBodyConfigs.size());
        for (auto* config : simulatedBodyConfigs)
        {
            newBodyHandles.emplace_back(AddSimulatedBody(config));
        }
        return newBodyHandles;
    }

    AzPhysics::SimulatedBody* JoltScene::GetSimulatedBodyFromHandle(AzPhysics::SimulatedBodyHandle bodyHandle)
    {
        if (bodyHandle == AzPhysics::InvalidSimulatedBodyHandle)
        {
            return nullptr;
        }

        AzPhysics::SimulatedBodyIndex index = AZStd::get<AzPhysics::HandleTypeIndex::Index>(bodyHandle);
        if (index < m_simulatedBodies.size()
            && m_simulatedBodies[index].first == AZStd::get<AzPhysics::HandleTypeIndex::Crc>(bodyHandle))
        {
            return m_simulatedBodies[index].second;
        }
        return nullptr;
    }

    AzPhysics::SimulatedBodyList JoltScene::GetSimulatedBodiesFromHandle(const AzPhysics::SimulatedBodyHandleList& bodyHandles)
    {
        AzPhysics::SimulatedBodyList results;
        for (auto& handle : bodyHandles)
        {
            results.emplace_back(GetSimulatedBodyFromHandle(handle));
        }
        return results;
    }

    void JoltScene::RemoveSimulatedBody(AzPhysics::SimulatedBodyHandle& bodyHandle)
    {
        if (bodyHandle == AzPhysics::InvalidSimulatedBodyHandle)
        {
            return;
        }

        AzPhysics::SimulatedBodyIndex index = AZStd::get<AzPhysics::HandleTypeIndex::Index>(bodyHandle);
        if (index < m_simulatedBodies.size()
            && m_simulatedBodies[index].first == AZStd::get<AzPhysics::HandleTypeIndex::Crc>(bodyHandle))
        {
            if (m_simulatedBodies[index].second->m_simulating)
            {
                // Disable simulation on body (not signaling OnSimulationBodySimulationDisabled event)
                DisableSimulationOfBodyInternal(*m_simulatedBodies[index].second);
            }

            m_simulatedBodyRemovedEvent.Signal(m_sceneHandle, bodyHandle);

            m_deferredDeletions.push_back(m_simulatedBodies[index].second);
            m_simulatedBodies[index] = AZStd::make_pair(AZ::Crc32(), nullptr);
            m_freeSceneSlots.push(index);

            bodyHandle = AzPhysics::InvalidSimulatedBodyHandle;
        }
    }

    void JoltScene::RemoveSimulatedBodies(AzPhysics::SimulatedBodyHandleList& bodyHandles)
    {
        for (auto& handle: bodyHandles)
        {
            RemoveSimulatedBody(handle);
        }
    }

    void JoltScene::EnableSimulationOfBody(AzPhysics::SimulatedBodyHandle bodyHandle)
    {
        if (bodyHandle == AzPhysics::InvalidSimulatedBodyHandle)
        {
            return;
        }

        if (AzPhysics::SimulatedBody* body = GetSimulatedBodyFromHandle(bodyHandle))
        {
            if (body->m_simulating)
            {
                return;
            }

            m_simulatedBodySimulationEnabledEvent.Signal(m_sceneHandle, bodyHandle);

            EnableSimulationOfBodyInternal(*body);
        }
        else
        {
            AZ_Warning("JoltScene", false, "Unable to enable Simulated body, failed to find body.")
        }
    }

    void JoltScene::DisableSimulationOfBody(AzPhysics::SimulatedBodyHandle bodyHandle)
    {
        if (bodyHandle == AzPhysics::InvalidSimulatedBodyHandle)
        {
            return;
        }

        if (AzPhysics::SimulatedBody* body = GetSimulatedBodyFromHandle(bodyHandle))
        {
            if (!body->m_simulating)
            {
                return;
            }

            m_simulatedBodySimulationDisabledEvent.Signal(m_sceneHandle, bodyHandle);

            DisableSimulationOfBodyInternal(*body);
        }
        else
        {
            AZ_Warning("JoltScene", false, "Unable to disable Simulated body, failed to find body.")
        }
    }

    AzPhysics::JointHandle JoltScene::AddJoint([[maybe_unused]] const AzPhysics::JointConfiguration* jointConfig,
    [[maybe_unused]] AzPhysics::SimulatedBodyHandle parentBody, [[maybe_unused]] AzPhysics::SimulatedBodyHandle childBody)
    {
        // TODO: Incomplete
        return AzPhysics::InvalidJointHandle;
    }

    AzPhysics::Joint* JoltScene::GetJointFromHandle(AzPhysics::JointHandle jointHandle)
    {
        if (jointHandle == AzPhysics::InvalidJointHandle)
        {
            return nullptr;
        }

        AzPhysics::JointIndex index = AZStd::get<AzPhysics::HandleTypeIndex::Index>(jointHandle);
        if (index < m_joints.size()
            && m_joints[index].first == AZStd::get<AzPhysics::HandleTypeIndex::Crc>(jointHandle))
        {
            return m_joints[index].second;
        }
        return nullptr;
    }

    void JoltScene::RemoveJoint(AzPhysics::JointHandle jointHandle)
    {
        if (jointHandle == AzPhysics::InvalidJointHandle)
        {
            return;
        }

        AzPhysics::JointIndex index = AZStd::get<AzPhysics::HandleTypeIndex::Index>(jointHandle);
        if (index < m_joints.size()
            && m_joints[index].first == AZStd::get<AzPhysics::HandleTypeIndex::Crc>(jointHandle))
        {
            m_deferredDeletionsJoints.push_back(m_joints[index].second);
            m_joints[index] = AZStd::make_pair(AZ::Crc32(), nullptr);
            m_freeJointSlots.push(index);
            jointHandle = AzPhysics::InvalidJointHandle;
        }
    }

    AzPhysics::SceneQueryHits JoltScene::QueryScene(const AzPhysics::SceneQueryRequest* request)
    {
        AzPhysics::SceneQueryHits hits;
        QueryScene(request, hits);
        return hits;
    }

    bool JoltScene::QueryScene([[maybe_unused]] const AzPhysics::SceneQueryRequest* request, [[maybe_unused]] AzPhysics::SceneQueryHits& result)
    {
        // TODO: Incomplete
        return false;
    }

    AzPhysics::SceneQueryHitsList JoltScene::QuerySceneBatch(const AzPhysics::SceneQueryRequests& requests)
    {
        AzPhysics::SceneQueryHitsList results;
        results.reserve(requests.size());
        for (auto& request : requests)
        {
            results.emplace_back(QueryScene(request.get()));
        }
        return results;
    }

    [[nodiscard]] bool JoltScene::QuerySceneAsync([[maybe_unused]] AzPhysics::SceneQuery::AsyncRequestId requestId,
        [[maybe_unused]] const AzPhysics::SceneQueryRequest* request, [[maybe_unused]] AzPhysics::SceneQuery::AsyncCallback callback)
    {
        AZ_Warning("Jolt", false, "Currently unimplemented.");
        return false;
    }

    [[nodiscard]] bool JoltScene::QuerySceneAsyncBatch([[maybe_unused]] AzPhysics::SceneQuery::AsyncRequestId requestId,
        [[maybe_unused]] const AzPhysics::SceneQueryRequests& requests, [[maybe_unused]] AzPhysics::SceneQuery::AsyncBatchCallback callback)
    {
        AZ_Warning("Jolt", false, "Currently unimplemented.");
        return false;
    }

    // Not sure if this has an equivalent in Jolt
    void JoltScene::SuppressCollisionEvents(const AzPhysics::SimulatedBodyHandle& bodyHandleA, const AzPhysics::SimulatedBodyHandle& bodyHandleB)
    {
        AzPhysics::SimulatedBody* bodyA = GetSimulatedBodyFromHandle(bodyHandleA);
        AzPhysics::SimulatedBody* bodyB = GetSimulatedBodyFromHandle(bodyHandleB);
        if (bodyA != nullptr && bodyB != nullptr)
        {
            // m_collisionFilterCallback.RegisterSuppressedCollision(bodyA, bodyB);
            AZ_Warning("Jolt", false, "Currently unimplemented.");
        }
    }

    // Not sure if this has an equivalent in Jolt
    void JoltScene::UnsuppressCollisionEvents(const AzPhysics::SimulatedBodyHandle& bodyHandleA, const AzPhysics::SimulatedBodyHandle& bodyHandleB)
    {
        AzPhysics::SimulatedBody* bodyA = GetSimulatedBodyFromHandle(bodyHandleA);
        AzPhysics::SimulatedBody* bodyB = GetSimulatedBodyFromHandle(bodyHandleB);
        if (bodyA != nullptr && bodyB != nullptr)
        {
            // m_collisionFilterCallback.UnregisterSuppressedCollision(bodyA, bodyB);
            AZ_Warning("Jolt", false, "Currently unimplemented.");
        }
    }

    void JoltScene::SetGravity(const AZ::Vector3& gravity)
    {
        if (m_physicsSystem && !m_gravity.IsClose(gravity))
        {
            m_gravity = gravity;
            {
                // Unsure if this is threadsafe
                m_physicsSystem->SetGravity(JoltMathConvert(gravity));
            }
            m_sceneGravityChangedEvent.Signal(m_sceneHandle, m_gravity);
        }
    }

    AZ::Vector3 JoltScene::GetGravity() const
    {
        return m_gravity;
    }

    void* JoltScene::GetNativePointer() const
    {
        return m_physicsSystem.get();
    }

    void JoltScene::FlushTransformSync()
    {
        AZ_PROFILE_SCOPE(Physics, "Jolt::FlushTransformSync");

        auto transformSync = [this](AzPhysics::SimulatedBodyIndex bodyIndex)
        {
            if (bodyIndex < m_simulatedBodies.size() && m_simulatedBodies[bodyIndex].second)
            {
                m_simulatedBodies[bodyIndex].second->SyncTransform(m_accumulatedDeltaTime);
            }
        };

        if (jolt_parallelTransformSync)
        {
            m_queuedActiveBodyIndices.ApplyParallel(transformSync, m_physicsSystem.get());
        }
        else
        {
            m_queuedActiveBodyIndices.Apply(transformSync);
        }

        m_queuedActiveBodyIndices.Clear();
        m_accumulatedDeltaTime = 0.0f;
    }

    void JoltScene::QueuedActiveBodyIndices::Apply(const AZStd::function<void(AzPhysics::SimulatedBodyIndex)>& applyFunction)
    {
        AZStd::for_each(m_packedIndices.begin(), m_packedIndices.end(), applyFunction);
    }

    void JoltScene::QueuedActiveBodyIndices::ApplyParallel(const AZStd::function<void(AzPhysics::SimulatedBodyIndex)>& applyFunction, JPH::PhysicsSystem* joltSystem)
    {
        AZ::TaskGraph taskGraph("Parallel Sync");
        AZ::TaskGraphEvent finishEvent("Parallel sync event");

        {
            AZ_PROFILE_SCOPE(Physics, "Sync Setup");

            size_t batchSize = jolt_parallelTransformSyncBatchSize;
            size_t fullSize = m_packedIndices.size();
            for (size_t i = 0; i < fullSize; i += batchSize)
            {
                AZ::TaskDescriptor taskDescriptor{"SyncTask", "Physics"};
                taskGraph.AddTask(
                    taskDescriptor,
                    [start = i, end = AZStd::min(i + batchSize, fullSize), &applyFunction, joltSystem, this]()
                    {
                        AZ_PROFILE_SCOPE(Physics, "Sync Task");
                        AZ_UNUSED(joltSystem)
                        // Note: It is important to keep the scene locked for read for the entire task execution.
                        // Otherwise the functions reading data from the rigid body will have to lock it locally.
                        // This causes a huge amount of context switches making the execution of each task ~20x slower.
                        // TODO: system is only here to lock read

                        for (size_t batchIndex = start; batchIndex < end; ++batchIndex)
                        {
                            applyFunction(m_packedIndices[batchIndex]);
                        }
                    });
            }

            taskGraph.Submit(&finishEvent);
        }

        finishEvent.Wait();
    }

    void JoltScene::QueuedActiveBodyIndices::Insert(AzPhysics::SimulatedBodyIndex bodyIndex)
    {
        if (m_uniqueIndices.insert(bodyIndex).second)
        {
            m_packedIndices.emplace_back(bodyIndex);
        }
    }

    void JoltScene::QueuedActiveBodyIndices::IncreaseCapacity(size_t extraSize)
    {
        m_packedIndices.reserve(m_packedIndices.size() + extraSize);
    }

    void JoltScene::QueuedActiveBodyIndices::Clear()
    {
        m_uniqueIndices.clear();
        m_packedIndices.clear();
    }

    void JoltScene::InitializeJoltSystem()
    {
        if (JoltSystem* system = GetJoltSystem())
        {
            // Setting user-defined settings for Physics System
            JPH::PhysicsSettings settings;
            settings.mNumVelocitySteps = m_cachedSystemConfig.m_systemInitSettings.m_numVelocitySteps;
            settings.mNumPositionSteps = m_cachedSystemConfig.m_systemInitSettings.m_numPositionSteps;
            settings.mAllowSleeping = m_cachedSystemConfig.m_systemInitSettings.m_allowSleeping;
            settings.mTimeBeforeSleep = m_cachedSystemConfig.m_systemInitSettings.m_timeBeforeSleep;
            settings.mPointVelocitySleepThreshold = m_cachedSystemConfig.m_systemInitSettings.m_pointVelocitySleepThreshold;
            settings.mDeterministicSimulation = m_cachedSystemConfig.m_systemInitSettings.m_deterministicSimulation;
            settings.mSpeculativeContactDistance = m_cachedSystemConfig.m_systemInitSettings.m_speculativeContactDistance;
            settings.mPenetrationSlop = m_cachedSystemConfig.m_systemInitSettings.m_penetrationSlop;
            settings.mBaumgarte = m_cachedSystemConfig.m_systemInitSettings.m_baumgarte;
            settings.mUseBodyPairContactCache = m_cachedSystemConfig.m_systemInitSettings.m_useBodyPairContactCache;
            settings.mBodyPairCacheMaxDeltaPositionSq = AZStd::pow(m_cachedSystemConfig.m_systemInitSettings.m_bodyPairCacheMaxDeltaPosition, 2.0f);
            settings.mBodyPairCacheCosMaxDeltaRotationDiv2 = m_cachedSystemConfig.m_systemInitSettings.m_bodyPairCacheCosMaxDeltaRotation / 2.0f;
            settings.mMinVelocityForRestitution = m_config.m_bounceThresholdVelocity;

            m_physicsSystem->SetPhysicsSettings(settings);

            m_physicsSystem->Init(
                m_cachedSystemConfig.m_systemInitSettings.m_maxBodies,
                m_cachedSystemConfig.m_systemInitSettings.m_numBodyMutexes,
                m_cachedSystemConfig.m_systemInitSettings.m_maxBodyPairs,
                m_cachedSystemConfig.m_systemInitSettings.m_maxContactConstraints,
                system->GetBroadPhaseLayerInterface(), system->GetObjectVsBroadPhaseLayerFilter(), system->GetObjectLayerPairFilter()
            );

            // TODO: maybe wrap these in a single sdk struct or something
            m_physicsSystem->SetContactListener(&m_contactListener);
            m_physicsSystem->SetBodyActivationListener(&m_activationListener);
            m_physicsSystem->SetGravity(JoltMathConvert(m_gravity));

            m_bodyInterface = &m_physicsSystem->GetBodyInterface();

            m_physicsSystem->OptimizeBroadPhase();
        }
    }

    void JoltScene::QueueBodyToAdd(JPH::Body* body, bool shouldActivate)
    {
        if (shouldActivate)
        {
            m_bodiesToAddAndActivate.push_back(body->GetID().GetIndexAndSequenceNumber());
        }
        else
        {
            m_bodiesToAdd.push_back(body->GetID().GetIndexAndSequenceNumber());
        }
    }

    bool JoltScene::RemoveBodyFromQueue(JPH::BodyID bodyId)
    {
        const AZ::u32 id = bodyId.GetIndexAndSequenceNumber();
        
        auto removeFromQueue = [](AZStd::vector<AZ::u32>& bodyVector, AZ::u32 bodyId)
        {
            auto it = AZStd::find(bodyVector.begin(), bodyVector.end(), bodyId);
            if (it != bodyVector.end())
            {
                AZStd::iter_swap(it, bodyVector.end() - 1);
                bodyVector.pop_back();
                return true;
            }
            return false;
        };
        
        const bool removed = removeFromQueue(m_bodiesToAdd, id) || removeFromQueue(m_bodiesToAddAndActivate, id);
        
        AZ_Warning("JoltScene::RemoveBodyFromQueue", removed, "Body %u was not found in any pending queue", bodyId);
        
        return removed;
    }

    void JoltScene::EnableSimulationOfBodyInternal(AzPhysics::SimulatedBody& body)
    {
        //character controller is a special actor and only needs the m_simulating flag set,
        // if (!azrtti_istypeof<JoltPhysics::CharacterController>(body) &&
        //     !azrtti_istypeof<JoltPhysics::Ragdoll>(body))
        {
            auto* joltBody = static_cast<JPH::Body*>(body.GetNativePointer());
            AZ_Assert(joltBody, "Simulated Body doesn't have a valid Jolt body");

            if (azrtti_istypeof<JoltPhysics::RigidBody>(body))
            {
                if (const auto rigidBody = azdynamic_cast<JoltPhysics::RigidBody*>(&body))
                {
                    QueueBodyToAdd(joltBody, !rigidBody->ShouldStartAsleep());
                }
            }
            else if (azrtti_istypeof<JoltPhysics::StaticRigidBody>(body))
            {
                if (const auto staticBody = azdynamic_cast<JoltPhysics::StaticRigidBody*>(&body))
                {
                    QueueBodyToAdd(joltBody, true);
                }
            }
        }

        body.m_simulating = true;
    }

    void JoltScene::DisableSimulationOfBodyInternal(AzPhysics::SimulatedBody& body)
    {
        //character controller is a special actor and only needs the m_simulating flag set,
        // if (!azrtti_istypeof<PhysX::CharacterController>(body) &&
        //     !azrtti_istypeof<PhysX::Ragdoll>(body) &&
        //     !azrtti_istypeof<PhysX::ArticulationLink>(body))
        if (auto* joltBody = static_cast<JPH::Body*>(body.GetNativePointer())) // TODO: temp fix to test basic shapes first
        {
            AZ_Assert(joltBody, "Simulated Body doesn't have a valid Jolt body");

            {
                m_bodyInterface->RemoveBody(joltBody->GetID());
            }
        }
        body.m_simulating = false;
    }

    void JoltScene::FlushQueuedEvents()
    {
        //send queued trigger events
        ProcessTriggerEvents();

        //send queued collision events
        ProcessCollisionEvents();
    }

    void JoltScene::ClearDeferredDeletions()
    {
        // swap the deletions in case the simulated body
        // manages more bodies and removes them on destruction (ie. Ragdoll).
        AZStd::vector<AzPhysics::SimulatedBody*> deletions;
        deletions.swap(m_deferredDeletions);
        for (auto* simulatedBody : deletions)
        {
            delete simulatedBody;
        }

        AZStd::vector<AzPhysics::Joint*> jointDeletions;
        jointDeletions.swap(m_deferredDeletionsJoints);
        for (auto* joint : jointDeletions)
        {
            delete joint;
        }
    }

    void JoltScene::ProcessTriggerEvents()
    {
        AZ_PROFILE_SCOPE(Physics, "JoltScene::ProcessTriggerEvents");

        AzPhysics::TriggerEventList& triggers = m_contactListener.GetQueuedTriggerEvents();
        if (triggers.empty())
        {
            return; // nothing to signal
        }
        m_sceneTriggerEvent.Signal(m_sceneHandle, triggers);

        for (auto& triggerEvent : triggers)
        {
            if (triggerEvent.m_triggerBody != nullptr)
            {
                triggerEvent.m_triggerBody->ProcessTriggerEvent(triggerEvent);
            }
            if (triggerEvent.m_otherBody != nullptr)
            {
                triggerEvent.m_otherBody->ProcessTriggerEvent(triggerEvent);
            }
        }

        //cleanup events for next simulate
        m_contactListener.FlushQueuedTriggerEvents();
    }

    void JoltScene::ProcessCollisionEvents()
    {
        AZ_PROFILE_SCOPE(Physics, "JoltScene::ProcessCollisionEvents");

        AzPhysics::CollisionEventList& collisions = m_contactListener.GetQueuedCollisionEvents();
        if (collisions.empty())
        {
            return; //nothing to signal
        }
        //send all event to any scene listeners
        m_sceneCollisionEvent.Signal(m_sceneHandle, collisions);

        //send events to each body listener
        for (auto& collision : collisions)
        {
            //trigger on body 1
            if (collision.m_body1 != nullptr)
            {
                collision.m_body1->ProcessCollisionEvent(collision);
            }
            //trigger for body 2
            if (collision.m_body2 != nullptr)
            {
                //swap the data as the event expects the trigger body to be body1.
                //this is ok to do as this event is no longer used after calling TriggerCollisionEvent
                AZStd::swap(collision.m_bodyHandle1, collision.m_bodyHandle2);
                AZStd::swap(collision.m_body1, collision.m_body2);
                AZStd::swap(collision.m_shape1, collision.m_shape2);
                collision.m_body1->ProcessCollisionEvent(collision);
            }
        }

        //cleanup events for next simulate
        m_contactListener.FlushQueuedCollisionEvents();
    }

    void JoltScene::UpdateAzProfilerDataPoints()
    {
        AZ_Warning("JoltScene::UpdateAzProfilerDataPoints", false, "Not currently implemented")
    }

    void JoltScene::SyncActiveBodyTransform(const AzPhysics::SimulatedBodyHandleList& activeBodyHandles)
    {
        if (auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get())
        {
            for (const AzPhysics::SimulatedBodyHandle& bodyHandle : activeBodyHandles)
            {
                if (AzPhysics::SimulatedBody* simBody = sceneInterface->GetSimulatedBodyFromHandle(m_sceneHandle, bodyHandle))
                {
                    simBody->SyncTransform(m_currentDeltaTime);
                }
            }
        }
    }
}
