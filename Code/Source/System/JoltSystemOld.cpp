
#include "JoltSystemOld.h"

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(JoltSystemOld, AZ::SystemAllocator);

    JoltSystemOld::JoltSystemOld(const JoltPhysics::SystemConfiguration& inConfig, const SystemHandle& inHandle)
        : System(inConfig)
        , m_config(inConfig)
        , m_handle(inHandle)
    {
        m_physicsSystem = AZStd::make_unique<JPH::PhysicsSystem>();
    }

    JoltSystemOld::~JoltSystemOld()
    {

    }

    void JoltSystemOld::StartUpdate(float deltatime)
    {
        m_physicsSystem->Update(deltatime, m_collisionSteps, m_tempAllocator, m_jobSystem);
    }

    void JoltSystemOld::FinishUpdate()
    {

    }

    // void JoltSystemOld::Initialize(JPH::uint inMaxBodies, JPH::uint inNumBodyMutexes, JPH::uint inMaxBodyPairs, JPH::uint inMaxContactConstraints, const JPH::BroadPhaseLayerInterface& inBroadPhaseLayerInterface, const JPH::ObjectVsBroadPhaseLayerFilter& inObjectVsBroadPhaseLayerFilter, const JPH::ObjectLayerPairFilter& inObjectLayerPairFilter)
    // {
    //     m_physicsSystem->Init(inMaxBodies, inNumBodyMutexes, inMaxBodyPairs, inMaxContactConstraints, inBroadPhaseLayerInterface, inObjectVsBroadPhaseLayerFilter, inObjectLayerPairFilter);
    //
    //     m_state = State::Initialized;
    // }

    const JPH::BodyInterface& JoltSystemOld::GetBodyInterface() const
    {
        return m_physicsSystem->GetBodyInterface();
    }

    AZ::Debug::PerformanceCollector* JoltSystemOld::GetPerformanceCollector() const
    {
        return m_performanceCollector.get();
    }


    JoltSystemOld* GetInternalJoltPhysicsSystem()
    {
        return azdynamic_cast<JoltSystemOld*>(AZ::Interface<JoltPhysics::SystemInterface>::Get());
    }
}
