
#pragma once

#include <AzFramework/Physics/Collision/CollisionEvents.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/SoftBody/SoftBodyContactListener.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

namespace JoltPhysics
{    
    enum class CombineMode : AZ::u8;
    

    /// The ContactListener class handles callbacks for all solid bodies. See SoftBodyContactListener for this purpose.
    class JoltContactListener : public JPH::ContactListener
    {
    public:
        virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override;

        virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;
        virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;
        virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

        /// Extract custom friction and restitution from a body and sub shape ID
        static void GetFrictionAndRestitution(const JPH::Body &inBody, const JPH::SubShapeID &inSubShapeID, float &outFriction, float &outRestitution, CombineMode& outFrictionCombine, CombineMode& outRestitutionCombine);

        /// Calculates and overrides friction and restitution settings for a contact between two bodies
        static void OverrideContactSettings(const JPH::Body &inBody1, const JPH::Body &inBody2, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings);

        //! Accessor to the queued Collision / trigger Events.
        AzPhysics::CollisionEventList& GetQueuedCollisionEvents();
        AzPhysics::TriggerEventList& GetQueuedTriggerEvents();

        //! Clear all queued collision / trigger events.
        void FlushQueuedCollisionEvents();
        void FlushQueuedTriggerEvents();

    private:
        void OnContact(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings, AzPhysics::CollisionEvent::Type inType);
        
        AzPhysics::CollisionEventList m_queuedCollisionEvents; //!< Holds all the collision events the happened until the next call to FlushCollisionEvents;
        AzPhysics::TriggerEventList m_queuedTriggerEvents; //!< Holds all the trigger events the happened until the next call to FlushTriggerEvents;
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////

    /// A listener class that receives collision contact events for soft bodies against rigid bodies.
    class JoltSoftBodyContactListener : public JPH::SoftBodyContactListener
    {
    public:
        virtual JPH::SoftBodyValidateResult OnSoftBodyContactValidate(const JPH::Body& inSoftBody, const JPH::Body& inOtherBody, JPH::SoftBodyContactSettings& ioSettings) override;

        virtual void OnSoftBodyContactAdded(const JPH::Body& inSoftBody, const JPH::SoftBodyManifold& inManifold) override;

        //! Accessor to the queued Collision / trigger Events.
        AzPhysics::CollisionEventList& GetQueuedCollisionEvents();

    private:
        AzPhysics::CollisionEventList m_queuedCollisionEvents; //!< Holds all the collision events the happened until the next call to FlushCollisionEvents;
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////
    
    /// An example activation listener
    class JoltBodyActivationListener : public JPH::BodyActivationListener
    {
    public:
        virtual void OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override;
        virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override;

        
    };
}
