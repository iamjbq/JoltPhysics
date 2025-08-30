
#include "JoltSystem.h"

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(JoltSystem, AZ::SystemAllocator);

    JoltSystem::JoltSystem(const JoltPhysics::SystemConfiguration& inConfig, const SystemHandle& inHandle)
        : System(inConfig)
        , m_config(inConfig)
        , m_handle(inHandle)
    {
        m_physicsSystem = AZStd::make_unique<JPH::PhysicsSystem>();
    }

    JoltSystem::~JoltSystem()
    {

    }

    void JoltSystem::StartUpdate(float deltatime)
    {
        m_physicsSystem->Update(deltatime, m_collisionSteps, m_tempAllocator, m_jobSystem);
    }

    void JoltSystem::FinishUpdate()
    {

    }

    // void JoltSystem::Initialize(JPH::uint inMaxBodies, JPH::uint inNumBodyMutexes, JPH::uint inMaxBodyPairs, JPH::uint inMaxContactConstraints, const JPH::BroadPhaseLayerInterface& inBroadPhaseLayerInterface, const JPH::ObjectVsBroadPhaseLayerFilter& inObjectVsBroadPhaseLayerFilter, const JPH::ObjectLayerPairFilter& inObjectLayerPairFilter)
    // {
    //     m_physicsSystem->Init(inMaxBodies, inNumBodyMutexes, inMaxBodyPairs, inMaxContactConstraints, inBroadPhaseLayerInterface, inObjectVsBroadPhaseLayerFilter, inObjectLayerPairFilter);
    //
    //     m_state = State::Initialized;
    // }

    const JPH::BodyInterface& JoltSystem::GetBodyInterface() const
    {
        return m_physicsSystem->GetBodyInterface();
    }

    AZ::Debug::PerformanceCollector* JoltSystem::GetPerformanceCollector() const
    {
        return m_performanceCollector.get();
    }


    JoltSystem* GetInternalJoltPhysicsSystem()
    {
        return azdynamic_cast<JoltSystem*>(AZ::Interface<JoltPhysics::SystemInterface>::Get());
    }
}
