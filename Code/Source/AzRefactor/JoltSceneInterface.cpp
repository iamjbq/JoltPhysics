
#include <AzRefactor/JoltSceneInterface.h>

#include <AzRefactor/JoltSystem.h>

#include <AzFramework/Physics/Common/PhysicsJoint.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
#include <AzFramework/Physics/Configuration/SceneConfiguration.h>

namespace JoltPhysics
{
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
} // JoltPhysics