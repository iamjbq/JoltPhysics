
#pragma once

#include <AzCore/Interface/Interface.h>

#include <JoltPhysics/JoltPhysicsTypes.h>
#include <System/JoltJobSystemThreaded.h>
#include <System/JoltSystem.h>

#include <System/WorldSimulationInterface.h>

namespace JPH
{
    class TempAllocatorImpl;
}

namespace JoltPhysics
{
    class WorldSimulationOwner final
        : public AZ::Interface<WorldSimulationInterface>::Registrar
    {
        public:
            AZ_CLASS_ALLOCATOR(WorldSimulationOwner, AZ::SystemAllocator);
            AZ_RTTI(WorldSimulationOwner, "{09E87EEA-6F73-449C-B341-8CD06A7E5406}");

            WorldSimulationOwner();
            virtual ~WorldSimulationOwner();

            //WorldSimulationInterface
            void Initialize() override;
            void Update() override;
            SystemHandle AddPhysicsSystem(const SystemConfiguration& config) override;

        private:
            const unsigned int AllocationArenaSize = 256 * 1024 * 1024;

            // All systems can share these as long as they are updated consecutively.
            // Considering moving to an O3DE allocator in the future if any benefits
            AZStd::shared_ptr<JPH::TempAllocatorImpl> m_allocator;
            AZStd::shared_ptr<JoltJobSystemThreaded> m_jobSystem;

            // This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
            // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
            const unsigned int m_maxBodies = 65536;

            // This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
            // mutexes are cheap! (they aren't)
            const unsigned int m_numBodyMutexes = 128;//for a LOT of reasons, we actually want the locks on the bodies to be quite granular.

            // This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
            // body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
            // too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
            // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
            const unsigned int m_maxBodyPairs = 65536;

            // This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
            // number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
            // number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
            // Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
            const unsigned int m_maxContactConstraints = 10240;

            JoltPhysics::SystemList m_systemList;

            // When a system is removed cache its index here to be used for the next
            // Ideally we won't be willy-nilly creating and destroying systems though
            AZStd::queue<JoltPhysics::SystemIndex> m_freeSystemSlots;

    };
} // JoltPhysics
