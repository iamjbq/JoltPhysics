
#include "JoltScene.h"

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
#include <Jolt/Core/JobSystemThreadPool.h>

#include <System/JoltSystem.h>
#include <Clients/RigidBody.h>
#include <JoltPhysics/MathConversions.h>
#include <System/CollisionLayerFilters.h>
#include <System/JoltJobSystemThreaded.h>
#include <System/ContactListener.h>
#include <System/BodyActivationListener.h>

namespace JoltPhysics
{
    namespace Internal
    {
        // TODO: Start implementing bodies - start with rigid body and work up to character controller

        // bool AddShape(AZStd::variant<AzPhysics::RigidBody*, AzPhysics::StaticRigidBody*> simulatedBody, const AzPhysics::ShapeVariantData& shapeData)
        // {
        //     if (const auto* shapeColliderPair = AZStd::get_if<AzPhysics::ShapeColliderPair>(&shapeData))
        //     {
        //         bool shapeAdded = false;
        //         auto shapePtr = AZStd::make_shared<Shape>(*(shapeColliderPair->first), *(shapeColliderPair->second));
        //         AZStd::visit([shapePtr, &shapeAdded](auto&& body)
        //             {
        //                 if (shapePtr->GetPxShape())
        //                 {
        //                     body->AddShape(shapePtr);
        //                     shapeAdded = true;
        //                 }
        //             }, simulatedBody);
        //         return shapeAdded;
        //     }
        //     else if (const auto* shapeColliderPairList = AZStd::get_if<AZStd::vector<AzPhysics::ShapeColliderPair>>(&shapeData))
        //     {
        //         bool shapeAdded = false;
        //         for (const auto& shapeColliderConfigs : *shapeColliderPairList)
        //         {
        //             auto shapePtr = AZStd::make_shared<Shape>(*(shapeColliderConfigs.first), *(shapeColliderConfigs.second));
        //             AZStd::visit([shapePtr, &shapeAdded](auto&& body)
        //                 {
        //                     if (shapePtr->GetPxShape())
        //                     {
        //                         body->AddShape(shapePtr);
        //                         shapeAdded = true;
        //                     }
        //                 }, simulatedBody);
        //         }
        //         return shapeAdded;
        //     }
        //     else if (const auto* shape = AZStd::get_if<AZStd::shared_ptr<Physics::Shape>>(&shapeData))
        //     {
        //         auto shapePtr = *shape;
        //         AZStd::visit([shapePtr](auto&& body)
        //             {
        //                 body->AddShape(shapePtr);
        //             }, simulatedBody);
        //         return true;
        //     }
        //     else if (const auto* shapeList = AZStd::get_if<AZStd::vector<AZStd::shared_ptr<Physics::Shape>>>(&shapeData))
        //     {
        //         for (auto shapePtr : *shapeList)
        //         {
        //             AZStd::visit([shapePtr](auto&& body)
        //                 {
        //                     body->AddShape(shapePtr);
        //                 }, simulatedBody);
        //         }
        //         return true;
        //     }
        //     return false;
        // }

        template<class SimulatedBodyType, class ConfigurationType>
        AzPhysics::SimulatedBody* CreateSimulatedBody(const ConfigurationType* configuration, AZ::Crc32& crc)
        {
            SimulatedBodyType* newBody = aznew SimulatedBodyType(*configuration);
            if (!AZStd::holds_alternative<AZStd::monostate>(configuration->m_colliderAndShapeData))
            {
                [[maybe_unused]] const bool shapeAdded = AddShape(newBody, configuration->m_colliderAndShapeData);
                AZ_Warning("JoltScene", shapeAdded, "No Collider or Shape information found when creating Rigid body [%s]", configuration->m_debugName.c_str());
            }
            crc = AZ::Crc32(newBody, sizeof(*newBody));
            return newBody;
        }

        // AzPhysics::SimulatedBody* CreateRigidBody(const AzPhysics::RigidBodyConfiguration* configuration, AZ::Crc32& crc)
        // {
        //     RigidBody* newBody = aznew RigidBody(*configuration);
        //     if (!AZStd::holds_alternative<AZStd::monostate>(configuration->m_colliderAndShapeData))
        //     {
        //         [[maybe_unused]] const bool shapeAdded = AddShape(newBody, configuration->m_colliderAndShapeData);
        //         AZ_Warning("JoltScene", shapeAdded, "No Collider or Shape information found when creating Rigid body [%s]", configuration->m_debugName.c_str());
        //     }
        //     const AzPhysics::MassComputeFlags& flags = configuration->GetMassComputeFlags();
        //     newBody->UpdateMassProperties(flags, configuration->m_centerOfMassOffset,
        //         configuration->m_inertiaTensor, configuration->m_mass);
        //
        //     crc = AZ::Crc32(newBody, sizeof(*newBody));
        //     return newBody;
        // }

    }

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

        if (JoltSystem* system = GetJoltSystem())
        {
            m_jobSystem = system->GetJoltJobSystem();
            m_tempAllocator = system->GetJoltAllocator();
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
                    // DisableSimulationOfBodyInternal(*simulatedBody.second);
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
        
        m_currentDeltaTime = deltaTime;
        m_physicsSystem->Update(deltaTime, m_collisionSteps, m_tempAllocator, m_jobSystem); // TODO: Find out why this is crashing
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
    }

    AzPhysics::SimulatedBodyHandle JoltScene::AddSimulatedBody([[maybe_unused]] const AzPhysics::SimulatedBodyConfiguration* simulatedBodyConfig)
    {
        // TODO: Incomplete
        // AzPhysics::SimulatedBody* newBody = nullptr;
        // AZ::Crc32 newBodyCrc;

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

    void JoltScene::RemoveSimulatedBody([[maybe_unused]] AzPhysics::SimulatedBodyHandle& bodyHandle)
    {
        // TODO: Incomplete
    }

    void JoltScene::RemoveSimulatedBodies(AzPhysics::SimulatedBodyHandleList& bodyHandles)
    {
        for (auto& handle: bodyHandles)
        {
            RemoveSimulatedBody(handle);
        }
    }

    void JoltScene::EnableSimulationOfBody([[maybe_unused]] AzPhysics::SimulatedBodyHandle bodyHandle)
    {
        // TODO: Incomplete
    }

    void JoltScene::DisableSimulationOfBody([[maybe_unused]] AzPhysics::SimulatedBodyHandle bodyHandle)
    {
        // TODO: Incomplete
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
    void JoltScene::SuppressCollisionEvents([[maybe_unused]] const AzPhysics::SimulatedBodyHandle& bodyHandleA, [[maybe_unused]] const AzPhysics::SimulatedBodyHandle& bodyHandleB)
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
    void JoltScene::UnsuppressCollisionEvents([[maybe_unused]] const AzPhysics::SimulatedBodyHandle& bodyHandleA, [[maybe_unused]] const AzPhysics::SimulatedBodyHandle& bodyHandleB)
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
                // TODO: Need to make sure this is safe. It may already be handled internally by Jolt
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

    void JoltScene::InitializeJoltSystem()
    {
        if (JoltSystem* system = GetJoltSystem()) // TODO: maybe change the nomenclature here to be less ambiguous
        {
            m_physicsSystem->Init(
                cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
                system->GetBroadPhaseLayerInterface(), system->GetObjectVsBroadPhaseLayerFilter(), system->GetObjectLayerPairFilter()
            );

            m_bodyInterface = &m_physicsSystem->GetBodyInterface();
            
            m_physicsSystem->OptimizeBroadPhase();
        }
    }

    // void JoltScene::FlushTransformSync()
    // {
    //
    // }
}
