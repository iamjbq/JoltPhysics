
#pragma once

#include <AzCore/Component/TickBus.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Debug/PerformanceCollector.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <System/SystemInterface.h>

namespace JoltPhysics
{
    class JoltPhysicsSystem final
        : public AZ::Interface<JoltPhysics::SystemInterface>::Registrar
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_RTTI(JoltPhysicsSystem, "{A843AAB0-3604-4DB9-B7B1-378E80869DF9}", JoltPhysics::SystemInterface);

        JoltPhysicsSystem();
        ~JoltPhysicsSystem() override = default;

        // SystemInterface interface ...
        void Initialize(JPH::uint inMaxBodies, JPH::uint inNumBodyMutexes, JPH::uint inMaxBodyPairs, JPH::uint inMaxContactConstraints, const JPH::BroadPhaseLayerInterface& inBroadPhaseLayerInterface, const JPH::ObjectVsBroadPhaseLayerFilter& inObjectVsBroadPhaseLayerFilter, const JPH::ObjectLayerPairFilter& inObjectLayerPairFilter) override;
        [[nodiscard]] const JPH::BodyInterface& GetBodyInterface() const;

        [[nodiscard]] AZ::Debug::PerformanceCollector* GetPerformanceCollector() const;

    private:
        AZStd::unique_ptr<JPH::PhysicsSystem> m_physicsSystem;
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

        // TODO: Could also be good guidance for anything with a Jolt equivalent
        ////////////////////////////////////////////////////////////////////////////
        // Debug::PhysXDebug m_physXDebug; //! Handler for the PhysXDebug Interface.
        // AZStd::unique_ptr<PhysXSettingsRegistryManager> m_registryManager; //! Handles all settings registry interactions.
        // PhysXSceneInterface m_sceneInterface; //! Implemented the Scene Az::Interface.
        // PhysXJointHelpersInterface m_jointHelperInterface; //! Implementation of the JointHelpersInterface.
        ////////////////////////////////////////////////////////////////////////////

        static constexpr AZStd::string_view PerformanceLogCategory = "Jolt Physics";
        static constexpr AZStd::string_view PerformanceSpecPhysXSimulationTime = "Jolt Simulation Time";

        AZStd::unique_ptr<AZ::Debug::PerformanceCollector> m_performanceCollector;
    };

    //! Helper function for getting the Jolt System interface from inside the Jolt Physics gem.
    JoltPhysicsSystem* GetInternalJoltPhysicsSystem();
}

