
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

	// Each broadPhase layer results in a separate bounding volume tree in the broad phase. You at least want to have
	// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
	// You can have a 1-on-1 mapping between object layers and broadPhase layers (like in this case) but if you have
	// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
	// your broadPhase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
	enum class JoltBroadPhaseLayer : AZ::u8
	{
		Static,
		Dynamic,
		Query,
		Trigger,
		Character,

		ENUM_COUNT
	  };

	inline AZ::u8 GetBroadPhaseCollisionMask(JoltBroadPhaseLayer broadPhase)
	{
	    // this mapping defines which types of objects can generally collide with each other
	    // if a flag is not included here, those types will never collide, no matter what their collision group is and other filter settings are
	    // note that this is only used for the simulation, ray casts and shape queries can use their own mapping

	    switch (broadPhase)
	    {
			case JoltBroadPhaseLayer::Static:
	    		return (1 << static_cast<AZ::u8>(JoltBroadPhaseLayer::Dynamic)) | (1 << static_cast<AZ::u8>(JoltBroadPhaseLayer::Character));

			case JoltBroadPhaseLayer::Dynamic:
	    		return (1 << static_cast<AZ::u8>(JoltBroadPhaseLayer::Static)) | (1 << static_cast<AZ::u8>(JoltBroadPhaseLayer::Dynamic)) | (1 << static_cast<AZ::u8>(JoltBroadPhaseLayer::Character)) | (1 << static_cast<AZ::u8>(JoltBroadPhaseLayer::Trigger));

	    	case JoltBroadPhaseLayer::Trigger:
	    		return (1 << static_cast<AZ::u8>(JoltBroadPhaseLayer::Dynamic)) | (1 << static_cast<AZ::u8>(JoltBroadPhaseLayer::Character));

		    // Queries should not interact with anything
		    case JoltBroadPhaseLayer::Query:
	    		return 0;

			case JoltBroadPhaseLayer::Character:
	    		return (1 << static_cast<AZ::u8>(JoltBroadPhaseLayer::Static)) | (1 << static_cast<AZ::u8>(JoltBroadPhaseLayer::Dynamic)) | (1 << static_cast<AZ::u8>(JoltBroadPhaseLayer::Character)) | (1 << static_cast<AZ::u8>(JoltBroadPhaseLayer::Trigger));

		    default: ;
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
			const AZ::u64 collisionLayer1 = 1ULL << static_cast<AZ::u8>(inObject1 >> 8);
        	const AZ::u64 collisionLayer2 = 1ULL << static_cast<AZ::u8>(inObject2 >> 8);

        	// The mask vector should be passed by ref to this class to eliminate this
			if (const JoltSystem* system = GetJoltSystem())
			{
				const AZ::u64 collisionMask1 = system->GetCollisionMask(inObject1 >> 16);
				const AZ::u64 collisionMask2 = system->GetCollisionMask(inObject2 >> 16);

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
		BPLayerInterfaceImpl() = default;

		[[nodiscard]] JPH::uint	GetNumBroadPhaseLayers() const override
		{
			return static_cast<JPH::uint>(JoltBroadPhaseLayer::ENUM_COUNT);
		}

		[[nodiscard]] JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			// We don't have a way to enforce this asset in the form that ObjectLayers are
			// JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
			return JPH::BroadPhaseLayer(static_cast<AZ::u8>(inLayer));
		}

	#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		[[nodiscard]] const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
		{
			switch (inLayer)
			{
			case Static:
				return "Static";

			case Dynamic:
				return "Dynamic";

			case Character:
				return "Character";

			case Query:
				return "Query";

			case Trigger:
				return "Trigger";

			default: ;
			}
		}
	#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:

	};

	/// Class that determines if an object layer can collide with a broadPhase layer
	class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		[[nodiscard]] bool ShouldCollide(const JPH::ObjectLayer inLayer1, const JPH::BroadPhaseLayer inLayer2) const override
		{
			// BroadPhase is stored as index in ObjectLayer
			const AZ::u8 mask1 = 1 << static_cast<AZ::u8>(inLayer1);
			const AZ::u8 mask2 = GetBroadPhaseCollisionMask(static_cast<JoltBroadPhaseLayer>((AZ::u8)inLayer2));

			return (mask1 & mask2) != 0;
		}
	};
}
