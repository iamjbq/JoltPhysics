
#include <AzCore/Debug/Trace.h>

#include <Scene/PhysicsSystemCallbacks.h>
#include <JoltPhysics/MathConversions.h>
#include <JoltPhysics/BodyData.h>

#include "Jolt/Physics/Body/Body.h"
#include "Jolt/Physics/Collision/EstimateCollisionResponse.h"

namespace JoltPhysics
{
    // PhysX set to 10, Jolt provides up to 64 points
    static constexpr const JPH::uint32 MaxPointsToReport = 64;
    
    // JoltContactListener TODO: Complete these functions
    
    JPH::ValidateResult JoltContactListener::OnContactValidate([[maybe_unused]] const JPH::Body& inBody1, [[maybe_unused]] const JPH::Body& inBody2,
        [[maybe_unused]] JPH::RVec3Arg inBaseOffset, [[maybe_unused]] const JPH::CollideShapeResult& inCollisionResult)
    {
        AZ_Info("JoltContactListener", "Contact validate callback")

        // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    void JoltContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
    {
        auto bodyData1 = reinterpret_cast<const BodyData*>(inBody1.GetUserData());
        auto bodyData2 = reinterpret_cast<const BodyData*>(inBody2.GetUserData());
        
        // Collision Event
        AzPhysics::CollisionEvent collision;
        collision.m_type = AzPhysics::CollisionEvent::Type::Begin;
        collision.m_bodyHandle1 = bodyData1->GetBodyHandle();
        collision.m_body1 = bodyData1->GetSimulatedBody();
        collision.m_bodyHandle2 = bodyData2->GetBodyHandle();
        collision.m_body2 = bodyData2->GetSimulatedBody();
        // collision.m_shape1 = ; // TODO
        // collision.m_shape2 = ;
        
        /// When contacts are added, the constraint solver has not run yet, so the collision impulse is unknown at that point.
        /// The velocities of inBody1 and inBody2 are the velocities before the contact has been resolved, so you can use this to
        /// estimate the collision impulse to e.g. determine the volume of the impact sound to play (see: EstimateCollisionResponse).
        JPH::CollisionEstimationResult estimate;
        JPH::EstimateCollisionResponse(inBody1, inBody2, inManifold, estimate, ioSettings.mCombinedFriction, ioSettings.mCombinedRestitution, 1.0f, 10);

        // Extract contacts for collision event
        // TODO: Need to sort out penetration depth != 0 where contact points on 1 != 2
        JPH::uint contactPointCount = AZStd::GetMax(inManifold.mRelativeContactPointsOn1.size(), inManifold.mRelativeContactPointsOn2.size()) ;
        JPH::uint pointsToReport = contactPointCount <= MaxPointsToReport ? contactPointCount : MaxPointsToReport;
        collision.m_contacts.resize(pointsToReport);
        for (JPH::uint i = 0; i < pointsToReport; ++i)
        {
            const JPH::Vec3 point1 = inManifold.GetWorldSpaceContactPointOn1(i);
            const JPH::Vec3 point2 = inManifold.GetWorldSpaceContactPointOn2(i);

            // Construct the impulse vector from constituents
            auto [mContactImpulse, mFrictionImpulse1, mFrictionImpulse2] = estimate.mImpulses.at(i);
            const JPH::Vec3 impulseVector = mContactImpulse * inManifold.mWorldSpaceNormal +
                                            mFrictionImpulse1 * estimate.mTangent1 +
                                            mFrictionImpulse2 * estimate.mTangent2;
            
            AzPhysics::Contact& contact = collision.m_contacts[i];
            contact.m_position = JoltMathConvert(point1 + point2) * 0.5f;
            contact.m_normal = JoltMathConvert(inManifold.mWorldSpaceNormal);
            contact.m_impulse = JoltMathConvert(impulseVector);
            contact.m_separation = inManifold.mPenetrationDepth; // TODO: double check this
        }

        m_queuedCollisionEvents.emplace_back(AZStd::move(collision));
        
        AZ_Info("JoltContactListener", "A contact was added")
    }

    void JoltContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
    {
        auto bodyData1 = reinterpret_cast<const BodyData*>(inBody1.GetUserData());
        auto bodyData2 = reinterpret_cast<const BodyData*>(inBody2.GetUserData());
        
        // Collision Event
        AzPhysics::CollisionEvent collision;
        collision.m_type = AzPhysics::CollisionEvent::Type::Persist;
        collision.m_bodyHandle1 = bodyData1->GetBodyHandle();
        collision.m_body1 = bodyData1->GetSimulatedBody();
        collision.m_bodyHandle2 = bodyData2->GetBodyHandle();
        collision.m_body2 = bodyData2->GetSimulatedBody();
        // collision.m_shape1 = ; // TODO
        // collision.m_shape2 = ;
        
        /// When contacts are added, the constraint solver has not run yet, so the collision impulse is unknown at that point.
        /// The velocities of inBody1 and inBody2 are the velocities before the contact has been resolved, so you can use this to
        /// estimate the collision impulse to e.g. determine the volume of the impact sound to play (see: EstimateCollisionResponse).
        JPH::CollisionEstimationResult estimate;
        JPH::EstimateCollisionResponse(inBody1, inBody2, inManifold, estimate, ioSettings.mCombinedFriction, ioSettings.mCombinedRestitution, 1.0f, 10);

        // Extract contacts for collision event
        // TODO: Need to sort out penetration depth != 0 where contact points on 1 != 2
        JPH::uint contactPointCount = AZStd::GetMax(inManifold.mRelativeContactPointsOn1.size(), inManifold.mRelativeContactPointsOn2.size()) ;
        JPH::uint pointsToReport = contactPointCount <= MaxPointsToReport ? contactPointCount : MaxPointsToReport;
        collision.m_contacts.resize(pointsToReport);
        for (JPH::uint i = 0; i < pointsToReport; ++i)
        {
            const JPH::Vec3 point1 = inManifold.GetWorldSpaceContactPointOn1(i);
            const JPH::Vec3 point2 = inManifold.GetWorldSpaceContactPointOn2(i);

            // Construct the impulse vector from constituents
            auto [mContactImpulse, mFrictionImpulse1, mFrictionImpulse2] = estimate.mImpulses.at(i);
            const JPH::Vec3 impulseVector = mContactImpulse * inManifold.mWorldSpaceNormal +
                                            mFrictionImpulse1 * estimate.mTangent1 +
                                            mFrictionImpulse2 * estimate.mTangent2;
            
            AzPhysics::Contact& contact = collision.m_contacts[i];
            contact.m_position = JoltMathConvert(point1 + point2) * 0.5f;
            contact.m_normal = JoltMathConvert(inManifold.mWorldSpaceNormal);
            contact.m_impulse = JoltMathConvert(impulseVector);
            contact.m_separation = inManifold.mPenetrationDepth; // TODO: double check this
        }

        m_queuedCollisionEvents.emplace_back(AZStd::move(collision));

        AZ_Info("JoltContactListener", "A contact persisted")
    }

    void JoltContactListener::OnContactRemoved([[maybe_unused]] const JPH::SubShapeIDPair& inSubShapePair)
    {
        // TODO: Need to treat this one a little differently
        
        AZ_Info("JoltContactListener", "A contact was removed")
    }

    AzPhysics::CollisionEventList& JoltContactListener::GetQueuedCollisionEvents()
    {
        return m_queuedCollisionEvents;
    }

    void JoltContactListener::FlushQueuedCollisionEvents()
    {
        m_queuedCollisionEvents.clear();
    }

    // JoltSoftBodyContactListener

    JPH::SoftBodyValidateResult JoltSoftBodyContactListener::OnSoftBodyContactValidate([[maybe_unused]] const JPH::Body& inSoftBody,
        [[maybe_unused]] const JPH::Body& inOtherBody, [[maybe_unused]] JPH::SoftBodyContactSettings& ioSettings)
    {
        return JPH::SoftBodyValidateResult::AcceptContact;
    }

    void JoltSoftBodyContactListener::OnSoftBodyContactAdded([[maybe_unused]] const JPH::Body& inSoftBody,
        [[maybe_unused]] const JPH::SoftBodyManifold& inManifold)
    {
        
    }

    // JoltBodyActivationListener

    void JoltBodyActivationListener::OnBodyActivated([[maybe_unused]] const JPH::BodyID& inBodyID, [[maybe_unused]] JPH::uint64 inBodyUserData)
    {
        AZ_Info("JoltBodyActivationListener", "A body got activated")
    }

    void JoltBodyActivationListener::OnBodyDeactivated([[maybe_unused]] const JPH::BodyID& inBodyID, [[maybe_unused]] JPH::uint64 inBodyUserData)
    {
        AZ_Info("JoltBodyActivationListener", "A body went to sleep")
    }
}
