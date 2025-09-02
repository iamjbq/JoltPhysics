
#pragma once

#include <AzCore/Component/TickBus.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Debug/PerformanceCollector.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <System/SystemInterface.h>

#include "Configuration/SystemConfiguration.h"
#include "JoltPhysics/JoltPhysicsTypes.h"

namespace JoltPhysics
{
    class JoltJobSystemThreaded;

    class JoltSystem
        : public JoltPhysics::System
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_RTTI(JoltSystem, "{A843AAB0-3604-4DB9-B7B1-378E80869DF9}", JoltPhysics::System);

        explicit JoltSystem(const JoltPhysics::SystemConfiguration& inConfig, const SystemHandle& inHandle);
        ~JoltSystem();

        void StartUpdate(float deltatime) override;
        void FinishUpdate() override;

        // void Initialize(JPH::uint inMaxBodies, JPH::uint inNumBodyMutexes, JPH::uint inMaxBodyPairs, JPH::uint inMaxContactConstraints, const JPH::BroadPhaseLayerInterface& inBroadPhaseLayerInterface, const JPH::ObjectVsBroadPhaseLayerFilter& inObjectVsBroadPhaseLayerFilter, const JPH::ObjectLayerPairFilter& inObjectLayerPairFilter) override;
        [[nodiscard]] const JPH::BodyInterface& GetBodyInterface() const;
        [[nodiscard]] AZ::Debug::PerformanceCollector* GetPerformanceCollector() const;

    private:
        AZStd::unique_ptr<JPH::PhysicsSystem> m_physicsSystem;
        JoltPhysics::SystemConfiguration m_config;
        JoltPhysics::SystemHandle m_handle;
        JPH::JobSystem* m_jobSystem;
        JPH::TempAllocatorImpl* m_tempAllocator;

        int m_collisionSteps = 1;
        
        // float m_accumulatedTime = 0.0f;

        // TODO: move this to worldSimulationOwner
        enum class State : AZ::u8
        {
            Uninitialized = 0,
            Initialized,
            Shutdown
        };
        State m_state = State::Uninitialized;

        static constexpr AZStd::string_view PerformanceLogCategory = "Jolt Physics";
        static constexpr AZStd::string_view PerformanceSpecPhysXSimulationTime = "Jolt Simulation Time";

        AZStd::unique_ptr<AZ::Debug::PerformanceCollector> m_performanceCollector;
    };

    //! Helper function for getting the Jolt System interface from inside the Jolt Physics gem.
    JoltSystem* GetInternalJoltPhysicsSystem();
}

