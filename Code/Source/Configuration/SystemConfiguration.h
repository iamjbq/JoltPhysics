
#pragma once

#include <AzCore/Math/Aabb.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Memory/Memory.h>
#include <AzCore/RTTI/TypeInfoSimple.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>

#include <JoltPhysics/JoltPhysicsTypes.h>

// #include <AzFramework/Physics/Configuration/CollisionConfiguration.h>
//
// #include "Jolt/Core/Core.h"
// #include <Jolt/Physics/Body/BodyInterface.h>
// #include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
// #include <Jolt/Physics/Collision/ContactListener.h>
// #include <Jolt/Physics/Constraints/ContactConstraintManager.h>
// #include <Jolt/Physics/Constraints/ConstraintManager.h>
// #include <Jolt/Physics/IslandBuilder.h>
// #include <Jolt/Physics/LargeIslandSplitter.h>
// #include <Jolt/Physics/PhysicsUpdateContext.h>
// #include <Jolt/Physics/PhysicsSettings.h>

namespace AZ
{
    class ReflectContext;
}

namespace JPH
{
    class JobSystem;
    class StateRecorder;
    class TempAllocator;
    class PhysicsStepListener;
    class SoftBodyContactListener;
    class SimShapeFilter;
}

namespace JoltPhysics
{
    struct alignas(16) SystemConfiguration
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_RTTI(JoltPhysics::SystemConfiguration, "{99B239C2-E6C3-495E-8582-1B6A61DB4A06}");
        static void Reflect(AZ::ReflectContext* context);

        static SystemConfiguration CreateDefault();
        virtual ~SystemConfiguration() = default;

        AZStd::string m_systemName;

        AZ::Aabb m_worldBounds = AZ::Aabb::CreateFromMinMax(-AZ::Vector3(1000.f, 1000.f, 1000.f), AZ::Vector3(1000.f, 1000.f, 1000.f));
        AZ::Vector3 m_gravity = JoltPhysics::DefaultGravity;

        static constexpr float DefaultFixedTimestep = 0.0166667f; //! Value represents 1/60th or 60 FPS.

        float m_maxTimestep = 0.1f; //!< Maximum fixed timestep in seconds to run the physics update (10FPS).
        float m_fixedTimestep = DefaultFixedTimestep; //!< Timestep in seconds to run the physics update. See DefaultFixedTimestep.

        // bool operator==(const SystemConfiguration& other) const;
        // bool operator!=(const SystemConfiguration& other) const;

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
    //! Alias for a list of SceneConfiguration objects, used for the creation of multiple Scenes at once.
    using SystemConfigurationList = AZStd::vector<SystemConfiguration>;
}

