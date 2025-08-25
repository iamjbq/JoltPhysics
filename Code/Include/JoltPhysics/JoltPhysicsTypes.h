
#pragma once

#include <AzCore/base.h>
#include <AzCore/RTTI/RTTI.h>

namespace JoltPhysics
{
    // Handle types
    using BodyId = uint32_t;
    using SystemId = uint32_t;
    using ShapeId = uint32_t;

    // Invalid handle constants
    static constexpr BodyId InvalidBodyId = AZStd::numeric_limits<BodyId>::max();
    static constexpr SystemId InvalidSystemId = AZStd::numeric_limits<SystemId>::max();

    // Motion types matching Jolt but AZ-style
    enum class MotionType : uint8_t
    {
        Static = 0,
        Kinematic,
        Dynamic
    };

    // Activation states
    enum class ActivationState : uint8_t
    {
        Active = 0,
        Inactive
    };

    // Object layers (might want more sophisticated layer management later)
    enum class ObjectLayer : uint16_t
    {
        Static = 0,
        Moving = 1,
        Debris = 2,
        // Add more as needed
    };

} // namespace JoltPhysics
