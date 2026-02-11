
#pragma once

#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/Common/PhysicsJoint.h>
#include <AzFramework/Physics/Common/PhysicsEvents.h>
#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>
#include <AzFramework/Physics/Configuration/SceneConfiguration.h>

#include <Scene/PhysicsSystemCallbacks.h>

#include <Jolt/Jolt.h>

namespace JoltPhysics
{
    class JoltAllocator;
}

namespace JPH
{
    class PhysicsSystem;
    class BodyInterface;
    class JobSystem;
    class TempAllocatorImpl;
    class JobSystemThreadPool;
}

namespace JoltPhysics
{
    class JoltJobSystemThreaded;
    class JoltAzAllocatorCallback;

    //! The Jolt implementation of a Physics Scene.
    //! Functionally it operates similar to a PhysX Scene, but under the hood is a Jolt Physics System
    //! containing a single simulation scene.
    class JoltScene final
        : public AzPhysics::Scene
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL
        AZ_RTTI(JoltScene, "{5F024E70-F590-4C6B-A150-996998F98D50}", AzPhysics::Scene)

        explicit JoltScene(const AzPhysics::SceneConfiguration& config, const AzPhysics::SceneHandle& sceneHandle);
        ~JoltScene();

        // AzPhysics::PhysicsScene ...
        void StartSimulation(float deltaTime) override;
        void FinishSimulation() override;
        void SetEnabled(bool enable) override;
        [[nodiscard]] bool IsEnabled() const override;
        [[nodiscard]] const AzPhysics::SceneConfiguration& GetConfiguration() const override;
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

        void InitializeJoltSystem();

    private:
        //! Data structure for efficient unique vector functionality.
        //! Body indices are inserted avoiding duplicated data and stored in a vector for efficient iteration.
        class QueuedActiveBodyIndices
        {
        public:
            void Insert(AzPhysics::SimulatedBodyIndex bodyIndex);
            void IncreaseCapacity(size_t extraSize);
            void Clear();
            void Apply(const AZStd::function<void(AzPhysics::SimulatedBodyIndex)>& applyFunction);
            void ApplyParallel(const AZStd::function<void(AzPhysics::SimulatedBodyIndex)>& applyFunction, JoltScene* joltScene);

        private:
            AZStd::unordered_set<AzPhysics::SimulatedBodyIndex> m_uniqueIndices;
            AZStd::vector<AzPhysics::SimulatedBodyIndex> m_packedIndices;
        };

        void EnableSimulationOfBodyInternal(AzPhysics::SimulatedBody& body);
        void DisableSimulationOfBodyInternal(AzPhysics::SimulatedBody& body);

        void FlushQueuedEvents();
        void ClearDeferredDeletions();
        void ProcessTriggerEvents();
        void ProcessCollisionEvents();

        void UpdateAzProfilerDataPoints();

        void SyncActiveBodyTransform(const AzPhysics::SimulatedBodyHandleList& activeBodyHandles);

        bool m_isEnabled = true;

        // Batch transform sync data. Here we store the indices of actors that have moved since the last simulation pass.
        // After the full simulation pass (possibly made of multiple simulation sub-steps) is complete,
        // we send the transform sync event once.
        QueuedActiveBodyIndices m_queuedActiveBodyIndices;

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

        AZStd::unique_ptr<JPH::PhysicsSystem> m_physicsSystem; //!< The underlying Jolt System
        JPH::BodyInterface* m_bodyInterface;
        JoltContactListener m_contactListener; //!< Callback class for collision event notification.
        JoltBodyActivationListener m_activationListener; //!< Callback class for body sleep/wake notification.

        // Cached variables to save look-up as they are inputs for every physics update loop
        // JoltJobSystemThreaded* m_jobSystem = nullptr;
        JPH::JobSystemThreadPool* m_jobSystem = nullptr;

        // JoltAzAllocatorCallback* m_tempAllocator = nullptr;
        JPH::TempAllocatorImpl* m_tempAllocator = nullptr;

        // System init variables
        unsigned int m_maxBodies = 65536;
        unsigned int m_numBodyMutexes = 128;
        unsigned int m_maxBodyPairs = 65536;
        unsigned int m_maxContactConstraints = 16384;
        int m_collisionSteps = 1;

        AZ::Vector3 m_gravity; // cache the gravity of the scene to avoid a lock in GetGravity().
    };
}
