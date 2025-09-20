
#pragma once


#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include "JoltSystem.h"
#include "AzCore/std/typetraits/add_const.h"

namespace AZ
{
	class ReflectContext;
}

namespace AzPhysics
{
	class CollisionGroup;
	class CollisionLayer;
	class CollisionConfiguration;
}

namespace JoltPhysics
{
	struct JoltSystemConfiguration;

	// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
	// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
	// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
	// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
	// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
	enum class JoltBroadPhaseLayer : AZ::u8
	{
		Static,
		Dynamic,
		Query,
		Trigger,
		Character,

		ENUM_COUNT
	  };

	AZ::u8 GetBroadphaseCollisionMask(JoltBroadPhaseLayer broadphase)
	{
	    // this mapping defines which types of objects can generally collide with each other
	    // if a flag is not included here, those types will never collide, no matter what their collision group is and other filter settings are
	    // note that this is only used for the simulation, raycasts and shape queries can use their own mapping

		// TODO: return bitmasks of all of these
	    switch (broadphase)
	    {
			case JoltBroadPhaseLayer::Static:
	    		return JoltBroadPhaseLayer::Dynamic | JoltBroadPhaseLayer::Character;

			case JoltBroadPhaseLayer::Dynamic:
	    		return JoltBroadPhaseLayer::Static | JoltBroadPhaseLayer::Character | JoltBroadPhaseLayer::Dynamic;

			case JoltBroadPhaseLayer::Character:
	    		return JoltBroadPhaseLayer::Static | JoltBroadPhaseLayer::Character | JoltBroadPhaseLayer::Dynamic;
	    }
	    return 0;
	};

    /// Class that determines if two object layers can collide
    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
    {
    public:
    	// ObjectLayer structure is 1-8: BroadPhaseLayer, 9-16: CollisionLayer idx, 17-24: CollisionGroup idx
        [[nodiscard]] bool ShouldCollide(const JPH::ObjectLayer inObject1, const JPH::ObjectLayer inObject2) const override
        {
			AZ::u64 collisionLayer1 = 1ULL << static_cast<AZ::u8>(inObject1 >> 8);
        	AZ::u64 collisionLayer2 = 1ULL << static_cast<AZ::u8>(inObject2 >> 8);

        	// The mask vector should be passed by ref to this class to eliminate this
			if (const JoltSystem* system = GetJoltSystem())
			{
				AZ::u64 collisionMask1 = system->GetCollisionMask(inObject1 >> 16);
				AZ::u64 collisionMask2 = system->GetCollisionMask(inObject2 >> 16);

				return (collisionMask1 & collisionLayer2) && (collisionMask2 & collisionLayer1);
			}
        	return false;
        }
    };

	// BroadPhaseLayerInterface implementation
	// This defines a mapping between object and broadphase layers.
	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl()
		{
			// Create a mapping table from object to broad phase layer

		}

		[[nodiscard]] JPH::uint	GetNumBroadPhaseLayers() const override
		{
			return static_cast<JPH::uint>(JoltBroadPhaseLayer::ENUM_COUNT);
		}

		[[nodiscard]] JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			// JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
			return JPH::BroadPhaseLayer(static_cast<AZ::u8>(inLayer));
		}

	#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		[[nodiscard]] const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
		{

		}
	#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:

	};

	/// Class that determines if an object layer can collide with a broadphase layer
	class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		[[nodiscard]] bool ShouldCollide(const JPH::ObjectLayer inLayer1, const JPH::BroadPhaseLayer inLayer2) const override
		{
			// GetBroadphaseCollisionMask(inLayer1)

			return false;
		}
	};
}
