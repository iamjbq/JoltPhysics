
#include "JoltPhysicsSystem.h"

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(JoltPhysicsSystem, AZ::SystemAllocator);

    JoltPhysicsSystem::JoltPhysicsSystem(const JoltPhysics::SystemConfiguration& inConfig, const SystemHandle& inHandle)
        : System(inConfig)
        , m_config(inConfig)
        , m_handle(inHandle)
    {
        m_physicsSystem = AZStd::make_unique<JPH::PhysicsSystem>();
    }

    JoltPhysicsSystem::~JoltPhysicsSystem()
    {

    }

    void JoltPhysicsSystem::StartUpdate(float deltatime)
    {
        m_physicsSystem->Update(deltatime, m_collisionSteps, m_tempAllocator, m_jobSystem);
    }

    void JoltPhysicsSystem::FinishUpdate()
    {

    }

    // void JoltPhysicsSystem::Initialize(JPH::uint inMaxBodies, JPH::uint inNumBodyMutexes, JPH::uint inMaxBodyPairs, JPH::uint inMaxContactConstraints, const JPH::BroadPhaseLayerInterface& inBroadPhaseLayerInterface, const JPH::ObjectVsBroadPhaseLayerFilter& inObjectVsBroadPhaseLayerFilter, const JPH::ObjectLayerPairFilter& inObjectLayerPairFilter)
    // {
    //     m_physicsSystem->Init(inMaxBodies, inNumBodyMutexes, inMaxBodyPairs, inMaxContactConstraints, inBroadPhaseLayerInterface, inObjectVsBroadPhaseLayerFilter, inObjectLayerPairFilter);
    //
    //     m_state = State::Initialized;
    // }

    const JPH::BodyInterface& JoltPhysicsSystem::GetBodyInterface() const
    {
        return m_physicsSystem->GetBodyInterface();
    }

    AZ::Debug::PerformanceCollector* JoltPhysicsSystem::GetPerformanceCollector() const
    {
        return m_performanceCollector.get();
    }


    JoltPhysicsSystem* GetInternalJoltPhysicsSystem()
    {
        return azdynamic_cast<JoltPhysicsSystem*>(AZ::Interface<JoltPhysics::SystemInterface>::Get());
    }
}
