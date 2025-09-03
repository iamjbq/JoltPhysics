
#pragma once

#include <AzCore/Component/TickBus.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Settings/SettingsRegistry.h>
#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Physics/Configuration/SystemConfiguration.h>

namespace AZ::Debug
{
    class PerformanceCollector;
}

namespace JoltPhysics
{
    class JoltSystem
        : public AZ::Interface<AzPhysics::SystemInterface>::Registrar
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_RTTI(JoltSystem, "{39383D29-C69C-4C36-84EE-874A3B5C9106}", AzPhysics::SystemInterface);

        JoltSystem();
        virtual ~JoltSystem();

        // SystemInterface interface ...
        void Initialize(const AzPhysics::SystemConfiguration* config) override;
        void Reinitialize() override;
        void Shutdown() override;
        void Simulate(float deltaTime) override;
        AzPhysics::SceneHandle AddScene(const AzPhysics::SceneConfiguration& config) override;
        AzPhysics::SceneHandleList AddScenes(const AzPhysics::SceneConfigurationList& configs) override;
        AzPhysics::SceneHandle GetSceneHandle(const AZStd::string& sceneName) override;
        AzPhysics::Scene* GetScene(AzPhysics::SceneHandle handle) override;
        AzPhysics::SceneList GetScenes(const AzPhysics::SceneHandleList& handles) override;
        AzPhysics::SceneList& GetAllScenes() override;
        void RemoveScene(AzPhysics::SceneHandle handle) override;
        void RemoveScenes(const AzPhysics::SceneHandleList& handles) override;
        void RemoveAllScenes() override;
        AZStd::pair<AzPhysics::SceneHandle, AzPhysics::SimulatedBodyHandle> FindAttachedBodyHandleFromEntityId(AZ::EntityId entityId) override;
        const AzPhysics::SystemConfiguration* GetConfiguration() const override;
        void UpdateConfiguration(const AzPhysics::SystemConfiguration* newConfig, bool forceReinitialization = false) override;
        void UpdateDefaultSceneConfiguration(const AzPhysics::SceneConfiguration& sceneConfiguration) override;
        const AzPhysics::SceneConfiguration& GetDefaultSceneConfiguration() const override;

        void SetCollisionLayerName(int index, const AZStd::string& layerName);
        void CreateCollisionGroup(const AZStd::string& groupName, const AzPhysics::CollisionGroup& group);
        //TEMP -- until these are fully moved over here

        AZ::Debug::PerformanceCollector* GetPerformanceCollector();

    private:
        AzPhysics::SceneConfiguration m_defaultSceneConfiguration;
        AzPhysics::SceneList m_sceneList;
        AZStd::queue<AzPhysics::SceneIndex> m_freeSceneSlots; //when a scene is removed cache its index here to be used for the next add.

        float m_accumulatedTime = 0.0f;

        enum class State : AZ::u8
        {
            Uninitialized = 0,
            Initialized,
            Shutdown
        };
        State m_state = State::Uninitialized;

        // TODO: Implement the JoltSceneInterface
        // PhysXSceneInterface m_sceneInterface; //! Implemented the Scene Az::Interface.

        static constexpr AZStd::string_view PerformanceLogCategory = "JoltPhysics";
        static constexpr AZStd::string_view PerformanceSpecPhysXSimulationTime = "Jolt Simulation Time";

        AZStd::unique_ptr<AZ::Debug::PerformanceCollector> m_performanceCollector;
    };
    //! Helper function for getting the Jolt System  from inside the Jolt Physics gem.
    JoltSystem* GetJoltSystem();
} // JoltPhysics
