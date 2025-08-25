
#pragma once

#include <AzCore/EBus/Event.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/functional.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/Common/PhysicsEvents.h>
#include <AzFramework/Physics/Common/PhysicsTypes.h>
#include <AzFramework/Physics/Configuration/SystemConfiguration.h>
#include <AzFramework/Physics/Configuration/SceneConfiguration.h>

#include <Jolt/Jolt.h>

namespace JPH
{
    class ObjectVsBroadPhaseLayerFilter;
    class BroadPhaseLayerInterface;
    class ObjectLayerPairFilter;
    class JobSystem;
    class StateRecorder;
    class TempAllocator;
    class PhysicsStepListener;
    class SoftBodyContactListener;
    class SimShapeFilter;
}

// #include <Configuration/SystemConfiguration.h>

namespace JoltPhysics
{
    class SystemInterface
    {
    public:
        AZ_RTTI(SystemInterface, "{6ACB39C4-514A-4263-B38E-5E81A32FE8FD}")

        SystemInterface() = default;
        virtual ~SystemInterface() = default;
        AZ_DISABLE_COPY_MOVE(SystemInterface)

        static void Reflect(AZ::ReflectContext* context);

        virtual void Initialize(JPH::uint inMaxBodies, JPH::uint inNumBodyMutexes, JPH::uint inMaxBodyPairs, JPH::uint inMaxContactConstraints, const JPH::BroadPhaseLayerInterface& inBroadPhaseLayerInterface, const JPH::ObjectVsBroadPhaseLayerFilter& inObjectVsBroadPhaseLayerFilter, const JPH::ObjectLayerPairFilter& inObjectLayerPairFilter) = 0;

    protected:
        // Just to remind me I may need to create event callbacks for Jolt too, or maybe I can use AzPhysics ones
        // AzPhysics::SystemEvents::OnInitializedEvent m_initializeEvent;
        // AzPhysics::SystemEvents::OnReinitializedEvent m_reinitializeEvent;
        // AzPhysics::SystemEvents::OnShutdownEvent m_shutdownEvent;
        // AzPhysics::SystemEvents::OnPresimulateEvent m_preSimulateEvent;
        // AzPhysics::SystemEvents::OnPostsimulateEvent m_postSimulateEvent;
        // AzPhysics::SystemEvents::OnSceneAddedEvent m_sceneAddedEvent;
        // AzPhysics::SystemEvents::OnSceneRemovedEvent m_sceneRemovedEvent;
        // AzPhysics::SystemEvents::OnConfigurationChangedEvent m_configChangeEvent;
        // AzPhysics::SystemEvents::OnDefaultSceneConfigurationChangedEvent m_onDefaultSceneConfigurationChangedEvent;        
    };
}

