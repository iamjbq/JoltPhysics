
#include "JoltPhysicsSystem.h"

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(JoltPhysicsSystem, AZ::SystemAllocator);

    JoltPhysicsSystem::JoltPhysicsSystem()
    {
        m_physicsSystem = AZStd::make_unique<JPH::PhysicsSystem>();
    }

    void JoltPhysicsSystem::Initialize(JPH::uint inMaxBodies, JPH::uint inNumBodyMutexes, JPH::uint inMaxBodyPairs, JPH::uint inMaxContactConstraints, const JPH::BroadPhaseLayerInterface& inBroadPhaseLayerInterface, const JPH::ObjectVsBroadPhaseLayerFilter& inObjectVsBroadPhaseLayerFilter, const JPH::ObjectLayerPairFilter& inObjectLayerPairFilter)
    {
        m_physicsSystem->Init(inMaxBodies, inNumBodyMutexes, inMaxBodyPairs, inMaxContactConstraints, inBroadPhaseLayerInterface, inObjectVsBroadPhaseLayerFilter, inObjectLayerPairFilter);

        m_state = State::Initialized;
    }

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
