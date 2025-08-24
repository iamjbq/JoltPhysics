
#include "JoltPhysicsSystem.h"

namespace JoltPhysics
{
    JoltPhysicsSystem::JoltPhysicsSystem()
    {
        m_physicsSystem = AZStd::make_unique<JPH::PhysicsSystem>();
    }

    JoltPhysicsSystem::~JoltPhysicsSystem()
    {
    }

    void JoltPhysicsSystem::Initialize([[maybe_unused]]JPH::uint inMaxBodies, [[maybe_unused]]JPH::uint inNumBodyMutexes, [[maybe_unused]]JPH::uint inMaxBodyPairs, [[maybe_unused]]JPH::uint inMaxContactConstraints, [[maybe_unused]]const JPH::BroadPhaseLayerInterface& inBroadPhaseLayerInterface, [[maybe_unused]]const JPH::ObjectVsBroadPhaseLayerFilter& inObjectVsBroadPhaseLayerFilter, [[maybe_unused]]const JPH::ObjectLayerPairFilter& inObjectLayerPairFilter)
    {
        // m_physicsSystem->Init(inMaxBodies, inNumBodyMutexes, inMaxBodyPairs, inMaxContactConstraints, inBroadPhaseLayerInterface, inObjectVsBroadPhaseLayerFilter, inObjectLayerPairFilter);

        m_state = State::Initialized;
    }

    const JPH::BodyInterface& JoltPhysicsSystem::GetBodyInterface() const
    {
        return m_physicsSystem->GetBodyInterface();
    }


    JoltPhysicsSystem* GetInternalJoltPhysicsSystem()
    {
        return azdynamic_cast<JoltPhysicsSystem*>(AZ::Interface<JoltPhysics::SystemInterface>::Get());
    }
}
