
#pragma once

#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/Common/PhysicsJoint.h>
#include <AzFramework/Physics/Common/PhysicsEvents.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
#include <AzFramework/Physics/Configuration/SceneConfiguration.h>

namespace JPH
{
    class PhysicsSystem;
    class JobSystem;
    class TempAllocatorImpl;
}

namespace JoltPhysics
{

    //! The Jolt implementation of a Physics Scene.
    //! Functionally it operates similar to a PhysX Scene, but under the hood is a Jolt Physics System
    //! containing a single simulation scene.
    class JoltPhysicsSystem final
        : public AzPhysics::Scene
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_RTTI(JoltPhysicsSystem, "{5F024E70-F590-4C6B-A150-996998F98D50}", AzPhysics::Scene);

        explicit JoltPhysicsSystem(const AzPhysics::SceneConfiguration& config, const AzPhysics::SceneHandle& sceneHandle);
        ~JoltPhysicsSystem();

        // AzPhysics::PhysicsScene ...
        void StartSimulation(float deltatime) override;
        void FinishSimulation() override;
        void SetEnabled(bool enable) override;
        bool IsEnabled() const override;
        const AzPhysics::SceneConfiguration& GetConfiguration() const override;
        void UpdateConfiguration(const AzPhysics::SceneConfiguration& config) override;
        AzPhysics::SimulatedBodyHandle AddSimulatedBody(const AzPhysics::SimulatedBodyConfiguration* simulatedBodyConfig) override;
        AzPhysics::SimulatedBodyHandleList AddSimulatedBodies(const AzPhysics::SimulatedBodyConfigurationList& simulatedBodyConfigs) override;
        AzPhysics::SimulatedBody* GetSimulatedBodyFromHandle(AzPhysics::SimulatedBodyHandle bodyHandle) override;
        AzPhysics::SimulatedBodyList GetSimulatedBodiesFromHandle(const AzPhysics::SimulatedBodyHandleList& bodyHandles) override;
        void RemoveSimulatedBody(AzPhysics::SimulatedBodyHandle& bodyHandle) override;
        void RemoveSimulatedBodies(AzPhysics::SimulatedBodyHandleList& bodyHandles) override;
        void EnableSimulationOfBody(AzPhysics::SimulatedBodyHandle bodyHandle) override;
        void DisableSimulationOfBody(AzPhysics::SimulatedBodyHandle bodyHandle) override;
        AzPhysics::JointHandle AddJoint(const AzPhysics::JointConfiguration* jointConfig,
            AzPhysics::SimulatedBodyHandle parentBody, AzPhysics::SimulatedBodyHandle childBody) override;
        AzPhysics::Joint* GetJointFromHandle(AzPhysics::JointHandle jointHandle) override;
        void RemoveJoint(AzPhysics::JointHandle jointHandle) override;
        AzPhysics::SceneQueryHits QueryScene(const AzPhysics::SceneQueryRequest* request) override;
        bool QueryScene(const AzPhysics::SceneQueryRequest* request, AzPhysics::SceneQueryHits& result) override;

        AzPhysics::SceneQueryHitsList QuerySceneBatch(const AzPhysics::SceneQueryRequests& requests) override;
        [[nodiscard]] bool QuerySceneAsync(AzPhysics::SceneQuery::AsyncRequestId requestId,
            const AzPhysics::SceneQueryRequest* request, AzPhysics::SceneQuery::AsyncCallback callback) override;
        [[nodiscard]] bool QuerySceneAsyncBatch(AzPhysics::SceneQuery::AsyncRequestId requestId,
            const AzPhysics::SceneQueryRequests& requests, AzPhysics::SceneQuery::AsyncBatchCallback callback) override;
        void SuppressCollisionEvents(
            const AzPhysics::SimulatedBodyHandle& bodyHandleA,
            const AzPhysics::SimulatedBodyHandle& bodyHandleB) override;
        void UnsuppressCollisionEvents(
            const AzPhysics::SimulatedBodyHandle& bodyHandleA,
            const AzPhysics::SimulatedBodyHandle& bodyHandleB) override;
        void SetGravity(const AZ::Vector3& gravity) override;
        AZ::Vector3 GetGravity() const override;

        AzPhysics::SceneHandle GetSceneHandle() const { return m_sceneHandle; }
        const AZStd::vector<AZStd::pair<AZ::Crc32, AzPhysics::SimulatedBody*>>& GetSimulatedBodyList() const { return m_simulatedBodies; }

        void* GetNativePointer() const override;

        //! Apply batched transform sync events for the current simulation pass.
        //! This will clear the batched data for the next simulation pass.
        void FlushTransformSync();

    private:

        bool m_isEnabled = true;

        // Accumulated delta time over multiple simulation sub-steps.
        // When we run the batched transform sync, the accumulated simulation time is provided
        // to tell how much time was simulated in this full pass.
        float m_accumulatedDeltaTime = 0.0f;

        AzPhysics::SceneConfiguration m_config;
        AzPhysics::SceneHandle m_sceneHandle;

        // Delta time for the current simulation sub-step
        float m_currentDeltaTime = 0.0f;

        AZStd::vector<AZStd::pair<AZ::Crc32, AzPhysics::SimulatedBody*>> m_simulatedBodies;
        AZStd::vector<AzPhysics::SimulatedBody*> m_deferredDeletions;
        AZStd::queue<AzPhysics::SimulatedBodyIndex> m_freeSceneSlots;

        AZStd::vector<AZStd::pair<AZ::Crc32, AzPhysics::Joint*>> m_joints;
        AZStd::vector<AzPhysics::Joint*> m_deferredDeletionsJoints;
        AZStd::queue<AzPhysics::JointIndex> m_freeJointSlots;

        AzPhysics::SystemEvents::OnConfigurationChangedEvent::Handler m_physicsSystemConfigChanged;

        AZ::u32 m_raycastBufferSize = 32; //!< Maximum number of hits that will be returned from a raycast.
        AZ::u32 m_shapecastBufferSize = 32; //!< Maximum number of hits that can be returned from a shapecast.
        AZ::u32 m_overlapBufferSize = 32; //!< Maximum number of overlaps that can be returned from an overlap query.

        AZStd::unique_ptr<JPH::PhysicsSystem> m_joltSystem; //!< The underlying Jolt System
        JPH::JobSystem* m_jobSystem;
        JPH::TempAllocatorImpl* m_tempAllocator;
        int m_collisionSteps = 1;

        AZ::Vector3 m_gravity; // cache the gravity of the scene to avoid a lock in GetGravity().
    };
}
