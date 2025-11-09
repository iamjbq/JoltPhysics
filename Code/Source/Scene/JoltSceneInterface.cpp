
#include <Scene/JoltSceneInterface.h>
#include <System/JoltSystem.h>

#include <AzFramework/Physics/Common/PhysicsJoint.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
#include <AzFramework/Physics/Configuration/SceneConfiguration.h>

namespace JoltPhysics
{
    namespace Internal
    {
        template<class Handler, class Function>
        void EventRegisterHelper(JoltSystem* joltSystem, AzPhysics::SceneHandle sceneHandle, Handler& handler, Function registerFunc)
        {
            if (AzPhysics::Scene* scene = joltSystem->GetScene(sceneHandle))
            {
                auto func = AZStd::bind(registerFunc, scene, AZStd::placeholders::_1);
                func(handler);
            }
        }
    }

    JoltSceneInterface::JoltSceneInterface(JoltSystem* inSystem)
        : m_joltSystem(inSystem)
    {
    }

    // AzPhysics::SceneInterface ...
    AzPhysics::SceneHandle JoltSceneInterface::GetSceneHandle(const AZStd::string& sceneName)
    {
        return m_joltSystem->GetSceneHandle(sceneName);
    }

    AzPhysics::Scene* JoltSceneInterface::GetScene(AzPhysics::SceneHandle handle)
    {
        return m_joltSystem->GetScene(handle);
    }

    void JoltSceneInterface::StartSimulation(AzPhysics::SceneHandle sceneHandle, float deltatime)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            scene->StartSimulation(deltatime);
        }
    }

    void JoltSceneInterface::FinishSimulation(AzPhysics::SceneHandle sceneHandle)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            scene->FinishSimulation();
        }
    }

    void JoltSceneInterface::SetEnabled(AzPhysics::SceneHandle sceneHandle, bool enable)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            scene->SetEnabled(enable);
        }
    }

    bool JoltSceneInterface::IsEnabled(AzPhysics::SceneHandle sceneHandle) const
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            return scene->IsEnabled();
        }
        return false;
    }

    AzPhysics::SimulatedBodyHandle JoltSceneInterface::AddSimulatedBody(AzPhysics::SceneHandle sceneHandle, const AzPhysics::SimulatedBodyConfiguration* simulatedBodyConfig)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            return scene->AddSimulatedBody(simulatedBodyConfig);
        }
        return AzPhysics::InvalidSimulatedBodyHandle;
    }

    AzPhysics::SimulatedBodyHandleList JoltSceneInterface::AddSimulatedBodies(AzPhysics::SceneHandle sceneHandle, const AzPhysics::SimulatedBodyConfigurationList& simulatedBodyConfigs)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            return scene->AddSimulatedBodies(simulatedBodyConfigs);
        }

        return {}; //return an empty list
    }

    AzPhysics::SimulatedBody* JoltSceneInterface::GetSimulatedBodyFromHandle(AzPhysics::SceneHandle sceneHandle, AzPhysics::SimulatedBodyHandle bodyHandle)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            return scene->GetSimulatedBodyFromHandle(bodyHandle);
        }
        return nullptr;
    }

    AzPhysics::SimulatedBodyList JoltSceneInterface::GetSimulatedBodiesFromHandle(AzPhysics::SceneHandle sceneHandle, const AzPhysics::SimulatedBodyHandleList& bodyHandles)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            return scene->GetSimulatedBodiesFromHandle(bodyHandles);
        }
        return {}; //return an empty list
    }

    void JoltSceneInterface::RemoveSimulatedBody(AzPhysics::SceneHandle sceneHandle, AzPhysics::SimulatedBodyHandle& bodyHandle)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            scene->RemoveSimulatedBody(bodyHandle);
        }
    }

    void JoltSceneInterface::RemoveSimulatedBodies(AzPhysics::SceneHandle sceneHandle, AzPhysics::SimulatedBodyHandleList& bodyHandles)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            scene->RemoveSimulatedBodies(bodyHandles);
        }
    }

    void JoltSceneInterface::EnableSimulationOfBody(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::SimulatedBodyHandle bodyHandle)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            scene->EnableSimulationOfBody(bodyHandle);
        }
    }

    void JoltSceneInterface::DisableSimulationOfBody(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::SimulatedBodyHandle bodyHandle)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            scene->DisableSimulationOfBody(bodyHandle);
        }
    }

    AzPhysics::JointHandle JoltSceneInterface::AddJoint(AzPhysics::SceneHandle sceneHandle,
        const AzPhysics::JointConfiguration* jointConfig, AzPhysics::SimulatedBodyHandle parentBody,
        AzPhysics::SimulatedBodyHandle childBody)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            return scene->AddJoint(jointConfig, parentBody, childBody);
        }

        return AzPhysics::InvalidJointHandle;
    }

    AzPhysics::Joint* JoltSceneInterface::GetJointFromHandle(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::JointHandle jointHandle)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            return scene->GetJointFromHandle(jointHandle);
        }

        return nullptr;
    }

    void JoltSceneInterface::RemoveJoint(AzPhysics::SceneHandle sceneHandle, AzPhysics::JointHandle jointHandle)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            scene->RemoveJoint(jointHandle);
        }
    }

    AzPhysics::SceneQueryHits JoltSceneInterface::QueryScene(AzPhysics::SceneHandle sceneHandle, const AzPhysics::SceneQueryRequest* request)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            return scene->QueryScene(request);
        }
        return AzPhysics::SceneQueryHits();
    }

    bool JoltSceneInterface::QueryScene(
        AzPhysics::SceneHandle sceneHandle, const AzPhysics::SceneQueryRequest* request, AzPhysics::SceneQueryHits& result)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            return scene->QueryScene(request, result);
        }
        return false;
    }

    AzPhysics::SceneQueryHitsList JoltSceneInterface::QuerySceneBatch(
        AzPhysics::SceneHandle sceneHandle, const AzPhysics::SceneQueryRequests& requests)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            return scene->QuerySceneBatch(requests);
        }
        return {}; //return an empty list
    }

    bool JoltSceneInterface::QuerySceneAsync(
        AzPhysics::SceneHandle sceneHandle, AzPhysics::SceneQuery::AsyncRequestId requestId,
        const AzPhysics::SceneQueryRequest* request, AzPhysics::SceneQuery::AsyncCallback callback)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            return scene->QuerySceneAsync(requestId, request, callback);
        }
        return false;
    }

    bool JoltSceneInterface::QuerySceneAsyncBatch(
        AzPhysics::SceneHandle sceneHandle, AzPhysics::SceneQuery::AsyncRequestId requestId,
        const AzPhysics::SceneQueryRequests& requests, AzPhysics::SceneQuery::AsyncBatchCallback callback)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            return scene->QuerySceneAsyncBatch(requestId, requests, callback);
        }
        return false;
    }

    void JoltSceneInterface::SuppressCollisionEvents(AzPhysics::SceneHandle sceneHandle,
        const AzPhysics::SimulatedBodyHandle& bodyHandleA,
        const AzPhysics::SimulatedBodyHandle& bodyHandleB)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            scene->SuppressCollisionEvents(bodyHandleA, bodyHandleB);
        }

    }

    void JoltSceneInterface::UnsuppressCollisionEvents(AzPhysics::SceneHandle sceneHandle,
        const AzPhysics::SimulatedBodyHandle& bodyHandleA,
        const AzPhysics::SimulatedBodyHandle& bodyHandleB)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            scene->UnsuppressCollisionEvents(bodyHandleA, bodyHandleB);
        }
    }

    void JoltSceneInterface::SetGravity(AzPhysics::SceneHandle sceneHandle, const AZ::Vector3& gravity)
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            scene->SetGravity(gravity);
        }
    }

    AZ::Vector3 JoltSceneInterface::GetGravity(AzPhysics::SceneHandle sceneHandle) const
    {
        if (AzPhysics::Scene* scene = m_joltSystem->GetScene(sceneHandle))
        {
            return scene->GetGravity();
        }
        return AZ::Vector3::CreateZero();
    }

    void JoltSceneInterface::RegisterSceneConfigurationChangedEventHandler(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::SceneEvents::OnSceneConfigurationChanged::Handler& handler)
    {
        Internal::EventRegisterHelper(m_joltSystem, sceneHandle, handler, &AzPhysics::Scene::RegisterSceneConfigurationChangedEventHandler);
    }
    
    void JoltSceneInterface::RegisterSimulationBodyAddedHandler(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::SceneEvents::OnSimulationBodyAdded::Handler& handler)
    {
        Internal::EventRegisterHelper(m_joltSystem, sceneHandle, handler, &AzPhysics::Scene::RegisterSimulationBodyAddedHandler);
    }

    void JoltSceneInterface::RegisterSimulationBodyRemovedHandler(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::SceneEvents::OnSimulationBodyRemoved::Handler& handler)
    {
        Internal::EventRegisterHelper(m_joltSystem, sceneHandle, handler, &AzPhysics::Scene::RegisterSimulationBodyRemovedHandler);
    }

    void JoltSceneInterface::RegisterSimulationBodySimulationEnabledHandler(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::SceneEvents::OnSimulationBodySimulationEnabled::Handler& handler)
    {
        Internal::EventRegisterHelper(m_joltSystem, sceneHandle, handler, &AzPhysics::Scene::RegisterSimulationBodySimulationEnabledHandler);
    }

    void JoltSceneInterface::RegisterSimulationBodySimulationDisabledHandler(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::SceneEvents::OnSimulationBodySimulationDisabled::Handler& handler)
    {
        Internal::EventRegisterHelper(m_joltSystem, sceneHandle, handler, &AzPhysics::Scene::RegisterSimulationBodySimulationDisabledHandler);
    }

    void JoltSceneInterface::RegisterSceneSimulationStartHandler(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::SceneEvents::OnSceneSimulationStartHandler& handler)
    {
        Internal::EventRegisterHelper(m_joltSystem, sceneHandle, handler, &AzPhysics::Scene::RegisterSceneSimulationStartHandler);
    }

    void JoltSceneInterface::RegisterSceneSimulationFinishHandler(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::SceneEvents::OnSceneSimulationFinishHandler& handler)
    {
        Internal::EventRegisterHelper(m_joltSystem, sceneHandle, handler, &AzPhysics::Scene::RegisterSceneSimulationFinishHandler);
    }

    void JoltSceneInterface::RegisterSceneActiveSimulatedBodiesHandler(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::SceneEvents::OnSceneActiveSimulatedBodiesEvent::Handler& handler)
    {
        Internal::EventRegisterHelper(m_joltSystem, sceneHandle, handler, &AzPhysics::Scene::RegisterSceneActiveSimulatedBodiesHandler);
    }

    void JoltSceneInterface::RegisterSceneCollisionEventHandler(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::SceneEvents::OnSceneCollisionsEvent::Handler& handler)
    {
        Internal::EventRegisterHelper(m_joltSystem, sceneHandle, handler, &AzPhysics::Scene::RegisterSceneCollisionEventHandler);
    }

    void JoltSceneInterface::RegisterSceneTriggersEventHandler(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::SceneEvents::OnSceneTriggersEvent::Handler& handler)
    {
        Internal::EventRegisterHelper(m_joltSystem, sceneHandle, handler, &AzPhysics::Scene::RegisterSceneTriggersEventHandler);
    }

    void JoltSceneInterface::RegisterSceneGravityChangedEvent(AzPhysics::SceneHandle sceneHandle,
        AzPhysics::SceneEvents::OnSceneGravityChangedEvent::Handler& handler)
    {
        Internal::EventRegisterHelper(m_joltSystem, sceneHandle, handler, &AzPhysics::Scene::RegisterSceneGravityChangedEvent);
    }
} // JoltPhysics