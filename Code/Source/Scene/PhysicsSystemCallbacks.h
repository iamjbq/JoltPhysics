
#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

namespace JoltPhysics
{
    // An example contact listener
    class JoltContactListener : public JPH::ContactListener
    {
    public:
        // See: ContactListener
        virtual JPH::ValidateResult	OnContactValidate(const JPH::Body &inBody1, const JPH::Body &inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult &inCollisionResult) override;

        virtual void OnContactAdded(const JPH::Body &inBody1, const JPH::Body &inBody2, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override;
        virtual void OnContactPersisted(const JPH::Body &inBody1, const JPH::Body &inBody2, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override;
        virtual void OnContactRemoved(const JPH::SubShapeIDPair &inSubShapePair) override;

        //! Accessor to the queued Collision / trigger Events.
        AzPhysics::CollisionEventList& GetQueuedCollisionEvents();
        // AzPhysics::TriggerEventList& GetQueuedTriggerEvents();

        //! Clear all queued collision / trigger events.
        void FlushQueuedCollisionEvents();
        // void FlushQueuedTriggerEvents();

    private:
        AzPhysics::CollisionEventList m_queuedCollisionEvents; //!< Holds all the collision events the happened until the next call to FlushCollisionEvents;
        // AzPhysics::TriggerEventList m_queuedTriggerEvents; //!< Holds all the trigger events the happened until the next call to FlushTriggerEvents;
    };

    // An example activation listener
    class JoltBodyActivationListener : public JPH::BodyActivationListener
    {
    public:
        virtual void OnBodyActivated(const JPH::BodyID &inBodyID, JPH::uint64 inBodyUserData) override;

        virtual void OnBodyDeactivated(const JPH::BodyID &inBodyID, JPH::uint64 inBodyUserData) override;
    };
}
