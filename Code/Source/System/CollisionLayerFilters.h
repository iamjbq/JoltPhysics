
#pragma once

#include <AzCore/base.h>
#include "AzCore/std/containers/fixed_vector.h"
#include "AzFramework/Physics/Collision/CollisionLayers.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>


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
	class JoltSystem;
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
		ObjectLayerPairFilterImpl();

    	// ObjectLayer structure is 1-8: BroadPhaseLayer, 9-16: CollisionLayer idx, 17-24: CollisionGroup idx
        [[nodiscard]] bool ShouldCollide(const JPH::ObjectLayer inObject1, const JPH::ObjectLayer inObject2) const override;

    private:
    	AZStd::fixed_vector<AZ::u64, AzPhysics::CollisionLayers::MaxCollisionLayers>* m_collisionGroupMasks;
    };

	// BroadPhaseLayerInterface implementation
	// This defines a mapping between object and broadPhase layers.
	class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	public:
		BroadPhaseLayerInterfaceImpl() = default;

		[[nodiscard]] JPH::uint	GetNumBroadPhaseLayers() const override
		{
			return static_cast<JPH::uint>(JoltBroadPhaseLayer::ENUM_COUNT);
		}

		[[nodiscard]] JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			JPH_ASSERT(inLayer != 0);
			return JPH::BroadPhaseLayer(static_cast<AZ::u8>(inLayer));
		}

	#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		[[nodiscard]] const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
		{
			switch (JoltBroadPhaseLayer layer = static_cast<JoltBroadPhaseLayer>(static_cast<AZ::u8>(inLayer)))
			{
			case JoltBroadPhaseLayer::Static:
				return "Static";

			case JoltBroadPhaseLayer::Dynamic:
				return "Dynamic";

			case JoltBroadPhaseLayer::Character:
				return "Character";

			case JoltBroadPhaseLayer::Query:
				return "Query";

			case JoltBroadPhaseLayer::Trigger:
				return "Trigger";

			default:
				return "Invalid";
			}
		}
	#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	};

	/// Class that determines if an object layer can collide with a broadPhase layer
	class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		ObjectVsBroadPhaseLayerFilterImpl() = default;

		[[nodiscard]] bool ShouldCollide(const JPH::ObjectLayer inLayer1, const JPH::BroadPhaseLayer inLayer2) const override
		{
			// BroadPhase is stored as index in ObjectLayer
			const AZ::u8 mask1 = 1 << static_cast<AZ::u8>(inLayer1);
			const AZ::u8 mask2 = GetBroadPhaseCollisionMask(static_cast<JoltBroadPhaseLayer>(static_cast<AZ::u8>(inLayer2)));

			return (mask1 & mask2) != 0;
		}
	};
}
