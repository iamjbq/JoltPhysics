
#include <System/ContactListener.h>

#include <AzCore/Debug/Trace.h>

namespace JoltPhysics
{
    JPH::ValidateResult JoltContactListener::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2,
        JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult)
    {
        AZ_Info("JoltContactListener", "Contact validate callback")

        // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    void JoltContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
    {
        AZ_Info("JoltContactListener", "A contact was added")
    }

    void JoltContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
    {
        AZ_Info("JoltContactListener", "A contact was persisted")
    }

    void JoltContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
    {
        AZ_Info("JoltContactListener", "A contact was removed")
    }
}
