
#pragma once

#include <AzCore/EBus/Event.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Physics/Common/PhysicsEvents.h>

#include <JoltPhysics/JoltPhysicsTypes.h>

#include <Jolt/Jolt.h>

namespace JoltPhysics
{
    class System;
    class JoltPhysicsSystem;

    struct SystemConfiguration;
}

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
    //! Interface to access a Physics System with a SystemHandle.
    class SystemInterface
    {
    public:
        AZ_RTTI(SystemInterface, "{6ACB39C4-514A-4263-B38E-5E81A32FE8FD}")

        SystemInterface() = default;
        virtual ~SystemInterface() = default;
        AZ_DISABLE_COPY_MOVE(SystemInterface)

        // virtual void Initialize(JPH::uint inMaxBodies, JPH::uint inNumBodyMutexes, JPH::uint inMaxBodyPairs, JPH::uint inMaxContactConstraints, const JPH::BroadPhaseLayerInterface& inBroadPhaseLayerInterface, const JPH::ObjectVsBroadPhaseLayerFilter& inObjectVsBroadPhaseLayerFilter, const JPH::ObjectLayerPairFilter& inObjectLayerPairFilter) = 0;

        //! Returns a System Handle connected to the given System name.
        //! @param SystemName The name of the System to look up.
        //! @returns Will return a SystemHandle to a System connected with the given name, otherwise will return InvalidSystemHandle.
        virtual SystemHandle GetSystemHandle(const AZStd::string& SystemName) = 0;

        //! Get the System of the requested SystemHandle.
        //! @param handle The SystemHandle of the requested System.
        //! @return Returns a System pointer if found, otherwise nullptr.
        virtual System* GetSystem(SystemHandle handle) = 0;

        //! Start the physics update process.
        //! As an example, this is a good place to trigger and queue any long-running work in separate threads.
        //! @param SystemHandle The SystemHandle of the System to use.
        //! @param deltatime The time in seconds to step the simulation for.
        virtual void StartUpdate(SystemHandle SystemHandle, float deltatime) = 0;

        //! Complete the update process.
        //! As an example, this is a good place to wait for any work to complete that was triggered in StartUpdate, or swap buffers if double buffering.
        //! @param SystemHandle The SystemHandle of the System to use.
        virtual void FinishUpdate(SystemHandle SystemHandle) = 0;

    };

    //! Interface of a Physics System
    class System
    {
        public:
            AZ_CLASS_ALLOCATOR_DECL;
            AZ_RTTI(System, "{CC1D29CB-4AD1-4A92-BA64-042C6433D913}");
            static void Reflect(AZ::ReflectContext* context);

            explicit System(const SystemConfiguration& config);
            virtual ~System() = default;

            //! Get the ID of the Scene.
            //! @return The Crc32 of the scene.
            const AZ::Crc32& GetId() const;

            //! Start the update process.
            //! As an example, this is a good place to trigger and queue any long-running work in separate threads.
            //! @param deltatime The time in seconds to run the update for.
            virtual void StartUpdate(float deltatime) = 0;

            //! Complete the update process.
            //! As an example, this is a good place to wait for any work to complete that was triggered in StartUpdate, or swap buffers if double buffering.
            virtual void FinishUpdate() = 0;

        protected:
            // TODO: Build Jolt versions of these events
            // AzPhysics::SystemEvents::OnInitializedEvent m_initializeEvent;
            // AzPhysics::SystemEvents::OnReinitializedEvent m_reinitializeEvent;
            // AzPhysics::SystemEvents::OnShutdownEvent m_shutdownEvent;
            // AzPhysics::SystemEvents::OnPresimulateEvent m_preSimulateEvent;
            // AzPhysics::SystemEvents::OnPostsimulateEvent m_postSimulateEvent;
            // AzPhysics::SystemEvents::OnSystemAddedEvent m_SystemAddedEvent;
            // AzPhysics::SystemEvents::OnSystemRemovedEvent m_SystemRemovedEvent;
            // AzPhysics::SystemEvents::OnConfigurationChangedEvent m_configChangeEvent;
            // AzPhysics::SystemEvents::OnDefaultSystemConfigurationChangedEvent m_onDefaultSystemConfigurationChangedEvent;

        private:
            AZ::Crc32 m_id;

    };
    //! Ease of use type for storing a list of System points
    using SystemList = AZStd::vector<AZStd::unique_ptr<System>>;
}

