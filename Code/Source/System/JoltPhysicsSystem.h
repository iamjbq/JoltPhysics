
#pragma once

#include <AzCore/Component/TickBus.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Debug/PerformanceCollector.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <System/SystemInterface.h>

#include "Configuration/SystemConfiguration.h"
#include "JoltPhysics/JoltPhysicsTypes.h"


// TODO: make a separate physics system interface class and leave PhysicsSystem for the world sim owner
namespace JoltPhysics
{
    class JoltPhysicsSystem
        : public JoltPhysics::System
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_RTTI(JoltPhysicsSystem, "{A843AAB0-3604-4DB9-B7B1-378E80869DF9}", JoltPhysics::System);

        explicit JoltPhysicsSystem(const JoltPhysics::SystemConfiguration& inConfig, const SystemHandle& inHandle);
        ~JoltPhysicsSystem();

        void StartUpdate(float deltatime) override;
        void FinishUpdate() override;

        // void Initialize(JPH::uint inMaxBodies, JPH::uint inNumBodyMutexes, JPH::uint inMaxBodyPairs, JPH::uint inMaxContactConstraints, const JPH::BroadPhaseLayerInterface& inBroadPhaseLayerInterface, const JPH::ObjectVsBroadPhaseLayerFilter& inObjectVsBroadPhaseLayerFilter, const JPH::ObjectLayerPairFilter& inObjectLayerPairFilter) override;
        [[nodiscard]] const JPH::BodyInterface& GetBodyInterface() const;
        [[nodiscard]] AZ::Debug::PerformanceCollector* GetPerformanceCollector() const;

    private:
        AZStd::unique_ptr<JPH::PhysicsSystem> m_physicsSystem;
        JoltPhysics::SystemConfiguration m_config;
        JoltPhysics::SystemHandle m_handle;
        // JoltJobSystemThreaded* m_jobSystem;
        // JPH::TempAllocatorImpl* m_tempAllocator;
        
        float m_accumulatedTime = 0.0f;

        // This seems like it could be useful from PhysX. We'll see if it applies to Jolt in the same way
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
    JoltPhysicsSystem* GetInternalJoltPhysicsSystem();
}

