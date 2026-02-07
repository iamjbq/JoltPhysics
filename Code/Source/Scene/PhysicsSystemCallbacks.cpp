
#include <AzCore/Debug/Trace.h>
#include <AzFramework/Physics/PhysicsScene.h>

#include <Scene/PhysicsSystemCallbacks.h>
#include <JoltPhysics/MathConversions.h>
#include <JoltPhysics/BodyData.h>
#include <JoltPhysics/Utils.h>

#include "Jolt/Physics/Body/Body.h"
#include "Jolt/Physics/Collision/EstimateCollisionResponse.h"

namespace JoltPhysics
{
    // PhysX set to 10, Jolt provides up to 64 points
    static constexpr const JPH::uint32 MaxPointsToReport = 10;

    void JoltContactListener::GetFrictionAndRestitution(const JPH::Body& inBody, const JPH::SubShapeID& inSubShapeID, float& outFriction,
        float& outRestitution, CombineMode& outFrictionCombine, CombineMode& outRestitutionCombine)
    {
        // Get the material that corresponds to the sub shape ID
        const JPH::PhysicsMaterial* material = inBody.GetShape()->GetMaterial(inSubShapeID);
        if (material == JPH::PhysicsMaterial::sDefault)
        {
            // This is the default material, use the settings from the body (no material was set)
            outFriction = inBody.GetFriction();
            outRestitution = inBody.GetRestitution();
        }
        else
        {
            // If it's not the default material we know it's a material that we created so we can cast it and get the values
            const auto* userMat = dynamic_cast<const JoltPhysicsMaterial*>(material);
            outFriction = userMat->GetFriction();
            outFrictionCombine = azdynamic_cast<JoltPhysics::Material*>(Utils::GetUserData(userMat))->GetFrictionCombineMode();
            outRestitution = userMat->GetRestitution();
            outRestitutionCombine = azdynamic_cast<JoltPhysics::Material*>(Utils::GetUserData(userMat))->GetRestitutionCombineMode();
        }
    }

    void JoltContactListener::OverrideContactSettings(const JPH::Body& inBody1, const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
    {
        // Get the custom friction and restitution for both bodies
        float friction1, friction2, restitution1, restitution2;
        CombineMode outFrictionCombine1, outFrictionCombine2, outRestitutionCombine1, outRestitutionCombine2;
        
        GetFrictionAndRestitution(inBody1, inManifold.mSubShapeID1, friction1, restitution1,
            outFrictionCombine1, outRestitutionCombine1);
        GetFrictionAndRestitution(inBody2, inManifold.mSubShapeID2, friction2, restitution2,
            outFrictionCombine2, outRestitutionCombine2);

        // Return the effective friction or restitution using max(mode1, mode2)
        ioSettings.mCombinedFriction = Utils::GetCombinedMaterialProperty(friction1, friction2, AZStd::max(outFrictionCombine1, outFrictionCombine2));
        ioSettings.mCombinedRestitution = Utils::GetCombinedMaterialProperty(restitution1, restitution2, AZStd::max(outRestitutionCombine1, outRestitutionCombine2));
    }

    JPH::ValidateResult JoltContactListener::OnContactValidate([[maybe_unused]] const JPH::Body& inBody1, [[maybe_unused]] const JPH::Body& inBody2,
                                                               [[maybe_unused]] JPH::RVec3Arg inBaseOffset, [[maybe_unused]] const JPH::CollideShapeResult& inCollisionResult)
    {
        // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    void JoltContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
    {
        OnContact(inBody1, inBody2, inManifold, ioSettings, AzPhysics::CollisionEvent::Type::Begin);
    }

    void JoltContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
    {
        OnContact(inBody1, inBody2, inManifold, ioSettings, AzPhysics::CollisionEvent::Type::Persist);
    }

    void JoltContactListener::OnContactRemoved([[maybe_unused]] const JPH::SubShapeIDPair& inSubShapePair)
    {
        // TODO: Need to treat this one a little differently
    }

    AzPhysics::CollisionEventList& JoltContactListener::GetQueuedCollisionEvents()
    {
        return m_queuedCollisionEvents;
    }

    AzPhysics::TriggerEventList& JoltContactListener::GetQueuedTriggerEvents()
    {
        return m_queuedTriggerEvents;
    }

    void JoltContactListener::FlushQueuedCollisionEvents()
    {
        m_queuedCollisionEvents.clear();
    }

    void JoltContactListener::FlushQueuedTriggerEvents()
    {
        m_queuedTriggerEvents.clear();
    }

    void JoltContactListener::OnContact(const JPH::Body& inBody1, const JPH::Body& inBody2,
                                        const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings,
                                        AzPhysics::CollisionEvent::Type inType)
    {
        // Override friction and restitution values before setting anything else
        OverrideContactSettings(inBody1, inBody2, inManifold, ioSettings);
        
        BodyData* bodyData1 = Utils::GetUserData(inBody1);
        BodyData* bodyData2 = Utils::GetUserData(inBody2);

        // Missing user data, or user data was invalid
        if (!bodyData1 || !bodyData2)
        {
            AZ_Warning("Jolt", false, "Invalid user data set for objects Obj0:%p Obj1:%p", bodyData1, bodyData2)
        }

        AzPhysics::SimulatedBody* body1 = bodyData1->GetSimulatedBody();
        AzPhysics::SimulatedBody* body2 = bodyData2->GetSimulatedBody();

        if (!body1 || !body2)
        {
            AZ_Warning("Jolt", false, "Invalid body data set for objects Obj0:%p Obj1:%p", body1, body2)
        }

        Physics::Shape* shape1 = Utils::GetUserData(inBody1.GetShape());
        Physics::Shape* shape2 = Utils::GetUserData(inBody2.GetShape());

        if (!shape1 || !shape2)
        {
            AZ_Warning("Jolt", false, "Invalid shape user data set for objects Obj0:%p Obj1:%p", shape1, shape2)
        }
        
        // Collision Event
        AzPhysics::CollisionEvent collision;
        collision.m_type = inType;
        collision.m_bodyHandle1 = bodyData1->GetBodyHandle();
        collision.m_body1 = body1;
        collision.m_bodyHandle2 = bodyData2->GetBodyHandle();
        collision.m_body2 = body2;
        collision.m_shape1 = shape1;
        collision.m_shape2 = shape2;
        
        /// When contacts are added, the constraint solver has not run yet, so the collision impulse is unknown at that point.
        /// The velocities of inBody1 and inBody2 are the velocities before the contact has been resolved, so you can use this to
        /// estimate the collision impulse to e.g. determine the volume of the impact sound to play (see: EstimateCollisionResponse).
        /// As per jrouwe, 4 iterations is more than sufficient for an accurate estimation of friction and impulses
        JPH::CollisionEstimationResult estimate;
        JPH::EstimateCollisionResponse(inBody1, inBody2, inManifold, estimate, ioSettings.mCombinedFriction, ioSettings.mCombinedRestitution, 1.0f, 4);

        // Extract contacts for collision event
        JPH::uint contactPointCount = AZStd::GetMax(inManifold.mRelativeContactPointsOn1.size(), inManifold.mRelativeContactPointsOn2.size()) ;
        JPH::uint pointsToReport = contactPointCount <= MaxPointsToReport ? contactPointCount : MaxPointsToReport;
        collision.m_contacts.resize(pointsToReport);
        for (JPH::uint i = 0; i < pointsToReport; ++i)
        {
            // If there is no penetration, these will be the same, but if so, they will be different
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
            contact.m_separation = inManifold.mPenetrationDepth; // Negative values are speculative and may not result in velocity change
            contact.m_internalFaceIndex01 = inManifold.mSubShapeID1.GetValue();
            contact.m_internalFaceIndex02 = inManifold.mSubShapeID2.GetValue();
        }

        m_queuedCollisionEvents.emplace_back(AZStd::move(collision));
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
        // TODO: Figure out what needs to be done here
    }

    AzPhysics::CollisionEventList& JoltSoftBodyContactListener::GetQueuedCollisionEvents()
    {
        return m_queuedCollisionEvents;
    }

    // JoltBodyActivationListener

    void JoltBodyActivationListener::OnBodyActivated([[maybe_unused]] const JPH::BodyID& inBodyID, [[maybe_unused]] JPH::uint64 inBodyUserData)
    {
        auto* bodyData = reinterpret_cast<JoltPhysics::BodyData*>(inBodyUserData);
        AZ_Printf("JoltBodyActivationListener", "Body %s was activated", bodyData->GetEntityId().ToString().c_str())
    }

    void JoltBodyActivationListener::OnBodyDeactivated([[maybe_unused]] const JPH::BodyID& inBodyID, [[maybe_unused]] JPH::uint64 inBodyUserData)
    {
        auto* bodyData = reinterpret_cast<JoltPhysics::BodyData*>(inBodyUserData);
        AZ_Printf("JoltBodyActivationListener", "Body %s was deactivated", bodyData->GetEntityId().ToString().c_str())
    }
}
