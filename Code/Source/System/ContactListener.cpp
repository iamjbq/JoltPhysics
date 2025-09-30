
#include <System/ContactListener.h>

#include <AzCore/Debug/Trace.h>

namespace JoltPhysics
{
    JPH::ValidateResult JoltContactListener::OnContactValidate([[maybe_unused]] const JPH::Body& inBody1, [[maybe_unused]] const JPH::Body& inBody2,
        [[maybe_unused]] JPH::RVec3Arg inBaseOffset, [[maybe_unused]] const JPH::CollideShapeResult& inCollisionResult)
    {
        AZ_Info("JoltContactListener", "Contact validate callback")

        // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    void JoltContactListener::OnContactAdded([[maybe_unused]] const JPH::Body& inBody1, [[maybe_unused]] const JPH::Body& inBody2,
        [[maybe_unused]] const JPH::ContactManifold& inManifold, [[maybe_unused]] JPH::ContactSettings& ioSettings)
    {
        AZ_Info("JoltContactListener", "A contact was added")
    }

    void JoltContactListener::OnContactPersisted([[maybe_unused]] const JPH::Body& inBody1, [[maybe_unused]] const JPH::Body& inBody2,
        [[maybe_unused]] const JPH::ContactManifold& inManifold, [[maybe_unused]] JPH::ContactSettings& ioSettings)
    {
        AZ_Info("JoltContactListener", "A contact was persisted")
    }

    void JoltContactListener::OnContactRemoved([[maybe_unused]] const JPH::SubShapeIDPair& inSubShapePair)
    {
        AZ_Info("JoltContactListener", "A contact was removed")
    }
}
