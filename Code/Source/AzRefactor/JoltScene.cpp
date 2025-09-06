
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

        m_joltSystem->Update(deltaTime, m_collisionSteps, m_tempAllocator, m_jobSystem);
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
