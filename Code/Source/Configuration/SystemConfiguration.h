
#pragma once

#include <AzCore/RTTI/RTTI.h>
#include <AzFramework/Physics/Configuration/CollisionConfiguration.h>

#include "Jolt/Core/Core.h"
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Constraints/ContactConstraintManager.h>
#include <Jolt/Physics/Constraints/ConstraintManager.h>
#include <Jolt/Physics/IslandBuilder.h>
#include <Jolt/Physics/LargeIslandSplitter.h>
#include <Jolt/Physics/PhysicsUpdateContext.h>
#include <Jolt/Physics/PhysicsSettings.h>

namespace AZ
{
    class ReflectContext;
}

JPH_NAMESPACE_BEGIN

class JobSystem;
class StateRecorder;
class TempAllocator;
class PhysicsStepListener;
class SoftBodyContactListener;
class SimShapeFilter;

JPH_NAMESPACE_END

namespace JoltPhysics
{
    struct alignas(16) SystemConfiguration
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_RTTI(JoltPhysics::SystemConfiguration, "{99B239C2-E6C3-495E-8582-1B6A61DB4A06}");
        static void Reflect(AZ::ReflectContext* context);

        /// Initialize the system.
        /// @param inMaxBodies Maximum number of bodies to support.
        /// @param inNumBodyMutexes Number of body mutexes to use. Should be a power of 2 in the range [1, 64], use 0 to auto-detect.
        /// @param inMaxBodyPairs Maximum amount of body pairs to process (anything else will fall through the world), this number should generally be much higher than the max amount of contact points as there will be lots of bodies close that are not actually touching.
        /// @param inMaxContactConstraints Maximum amount of contact constraints to process (anything else will fall through the world).
        /// @param inBroadPhaseLayerInterface Information on the mapping of object layers to broad phase layers. Since this is a virtual interface, the instance needs to stay alive during the lifetime of the PhysicsSystem.
        /// @param inObjectVsBroadPhaseLayerFilter Filter callback function that is used to determine if an object layer collides with a broad phase layer. Since this is a virtual interface, the instance needs to stay alive during the lifetime of the PhysicsSystem.
        /// @param inObjectLayerPairFilter Filter callback function that is used to determine if two object layers collide. Since this is a virtual interface, the instance needs to stay alive during the lifetime of the PhysicsSystem.
        SystemConfiguration(JPH::uint inMaxBodies, JPH::uint inNumBodyMutexes, JPH::uint inMaxBodyPairs, JPH::uint inMaxContactConstraints, const JPH::BroadPhaseLayerInterface &inBroadPhaseLayerInterface, const JPH::ObjectVsBroadPhaseLayerFilter &inObjectVsBroadPhaseLayerFilter, const JPH::ObjectLayerPairFilter &inObjectLayerPairFilter);
        virtual ~SystemConfiguration() = default;

        static constexpr float DefaultFixedTimestep = 0.0166667f; //! Value represents 1/60th or 60 FPS.

        float m_maxTimestep = 0.1f; //!< Maximum fixed timestep in seconds to run the physics update (10FPS).
        float m_fixedTimestep = DefaultFixedTimestep; //!< Timestep in seconds to run the physics update. See DefaultFixedTimestep.

        bool operator==(const SystemConfiguration& other) const;
        bool operator!=(const SystemConfiguration& other) const;

    private:
        // helpers for edit context
        AZ::u32 OnMaxTimeStepChanged();
        float GetFixedTimeStepMax() const;

        // Padding with a 16 byte aligned structure to make SystemConfiguration aligned to 16 bytes too.
        // Without this, SystemConfiguration generates warnings everywhere it is used indicating that
        // padding was added. But having this structure limits the warnings to this member usage because
        // SystemConfiguration won't need extra padding to achieve 16 byte alignment.
        AZ_PUSH_DISABLE_WARNING(4324, "-Wunknown-warning-option") // structure was padded due to alignment
        [[maybe_unused]] struct alignas(16)
        {
            unsigned char m_unused[16];
        } m_unusedPadding;
        AZ_POP_DISABLE_WARNING    
    };
}

