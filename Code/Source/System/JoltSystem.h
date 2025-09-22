
#pragma once

#include <AzCore/Component/TickBus.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Settings/SettingsRegistry.h>
#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Physics/Configuration/SystemConfiguration.h>

#include <Scene/JoltSceneInterface.h>
#include <JoltPhysics/Configuration/JoltConfiguration.h>
// #include <System/CollisionLayerFilters.h>

namespace AZ::Debug
{
    class PerformanceCollector;
}

namespace JPH
{
    class TempAllocatorImpl;
}

namespace JoltPhysics
{
    class JoltJobSystemThreaded;

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

        AZ::Debug::PerformanceCollector* GetPerformanceCollector();

        // Convenience functions to get the actual index of a collision group
        AZ::u32 GetCollisionGroupIndex(const AzPhysics::CollisionGroup& group) const;
        AZ::u32 GetCollisionGroupIndex(const AzPhysics::CollisionGroups::Id & groupId) const;

        // System may not be the best place to access this, but let's start here
        AZ::u64 GetCollisionMask(AZ::u32 index) const;

    private:
        // AZStd::fixed_vector<AZ::u64, AzPhysics::CollisionLayers::MaxCollisionLayers> m_collisionGroupMasks;
        AZ::u64 m_collisionGroupMasks[AzPhysics::CollisionLayers::MaxCollisionLayers];

        JoltSystemConfiguration m_systemConfig;
        AzPhysics::SceneConfiguration m_defaultSceneConfiguration;
        AzPhysics::SceneList m_sceneList;
        AZStd::queue<AzPhysics::SceneIndex> m_freeSceneSlots; //when a scene is removed cache its index here to be used for the next add.

        float m_accumulatedTime = 0.0f;

        // Collision filtering objects shared with all scenes
        // JoltPhysics::BPLayerInterfaceImpl m_broadPhaseInterface;
        // JoltPhysics::ObjectVsBroadPhaseLayerFilterImpl m_broadPhaseLayerFilter;
        // JoltPhysics::ObjectLayerPairFilterImpl m_objectLayerPairFilter;

        enum class State : AZ::u8
        {
            Uninitialized = 0,
            Initialized,
            Shutdown
        };
        State m_state = State::Uninitialized;

        // All systems can share these as long as they are updated consecutively.
        // Considering moving to an O3DE allocator in the future if any benefits
        AZStd::shared_ptr<JPH::TempAllocatorImpl> m_allocator;
        AZStd::shared_ptr<JoltJobSystemThreaded> m_jobSystem;

        JoltSceneInterface m_sceneInterface; //! Implement the Scene Az::Interface.

        static constexpr AZStd::string_view PerformanceLogCategory = "JoltPhysics";
        static constexpr AZStd::string_view PerformanceSpecJoltSimulationTime = "Jolt Simulation Time";

        AZStd::unique_ptr<AZ::Debug::PerformanceCollector> m_performanceCollector;
    };

    //! Helper function for getting the Jolt System from inside the Jolt Physics gem.
    JoltSystem* GetJoltSystem();
} // JoltPhysics
