
#include <System/CollisionLayerFilters.h>
#include <System/JoltSystem.h>

namespace JoltPhysics
{
    ObjectLayerPairFilterImpl::ObjectLayerPairFilterImpl()
    {
        m_collisionGroupMasks = GetJoltSystem()->GetCollisionMasks(); // this does compile
    }

    bool ObjectLayerPairFilterImpl::ShouldCollide(const JPH::ObjectLayer inObject1, const JPH::ObjectLayer inObject2) const
    {
        const AZ::u64 collisionLayer1 = 1ULL << static_cast<AZ::u8>(inObject1 >> 8);
        const AZ::u64 collisionLayer2 = 1ULL << static_cast<AZ::u8>(inObject2 >> 8);

        if (m_collisionGroupMasks != nullptr)
        {
            const AZ::u64 collisionMask1 = m_collisionGroupMasks->at(inObject1 >> 16);
            const AZ::u64 collisionMask2 = m_collisionGroupMasks->at(inObject2 >> 16);

            return (collisionMask1 & collisionLayer2) && (collisionMask2 & collisionLayer1);
        }
        AZ_Warning("ObjectLayerPairFilterImpl", false, "JoltSystem pointer was null.")
        return false;
    }
}
