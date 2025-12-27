
#include <System/CollisionLayerFilters.h>
#include <System/JoltSystem.h>

namespace JoltPhysics
{
    ObjectLayerPairFilterImpl::ObjectLayerPairFilterImpl()
    {
        if (JoltSystem* system = GetJoltSystem())
        {
            m_collisionGroupMasks = system->GetCollisionMasks();
        }
        else
        {
            AZ_Error("ObjectLayerPairFilterImpl", false, "Failed to set collision masks")
        }
    }

    bool ObjectLayerPairFilterImpl::ShouldCollide(const JPH::ObjectLayer inObject1, const JPH::ObjectLayer inObject2) const
    {
        const AZ::u64 collisionLayer1 = 1ULL << static_cast<AZ::u8>(inObject1 >> 8);
        const AZ::u64 collisionLayer2 = 1ULL << static_cast<AZ::u8>(inObject2 >> 8);

        if (m_collisionGroupMasks != nullptr)
        {
            AZ::u64 collisionMask1 = m_collisionGroupMasks->at(inObject1 >> 16);
            AZ::u64 collisionMask2 = m_collisionGroupMasks->at(inObject2 >> 16);

            return (collisionMask1 & collisionLayer2) && (collisionMask2 & collisionLayer1);
        }
        AZ_Warning("ObjectLayerPairFilterImpl", false, "JoltSystem pointer was null.")
        return false;
    }
}
