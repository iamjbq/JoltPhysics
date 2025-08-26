
#pragma once

#include <AzCore/base.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Math/Crc.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/tuple.h>
#include <AzCore/std/containers/variant.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <AzCore/Casting/numeric_cast.h>

namespace Physics
{
    class ColliderConfiguration;
    class Shape;
    class ShapeConfiguration;
}

namespace JoltPhysics
{
    // Shamelessly re-using as much AzPhysics code as possible to make my life easier.
    // Jolt Physics supports multiple systems that each own one scene and set of bodies.
    // So Jolt PhysicsSystem operates more like a PhysXScene.

    //! Maximum number of systems, Default System Names and Crc32
    constexpr AZ::u32 MaxNumberOfSystems = 64; // I may come to regret this not being like, 8 or 16, at most.
    constexpr const char* DefaultPhysicsSystemName = "DefaultSystem";
    constexpr AZ::Crc32 DefaultPhysicsSystemId = AZ_CRC_CE(DefaultPhysicsSystemName);
    constexpr const char* EditorPhysicsSystemName = "EditorSystem";
    constexpr AZ::Crc32 EditorPhysicsSystemId = AZ_CRC_CE(EditorPhysicsSystemName);

    //! Default gravity.
    static const AZ::Vector3 DefaultGravity = AZ::Vector3(0.0f, 0.0f, -9.81f);

    enum HandleTypeIndex
    {
        Crc = 0,
        Index
    };

    using SystemIndex = AZ::s8;
    using SimulatedBodyIndex = AZ::s32;
    using JointIndex = AZ::s32;
    static_assert(std::is_signed_v<SystemIndex>
        && std::is_signed_v<SimulatedBodyIndex>
        && std::is_signed_v<JointIndex>, "SystemIndex, SimulatedBodyIndex and JointIndex must be signed integers.");

    //! A handle to a System within the physics world simulation owner.
    //! A SystemHandle is a tuple of a Crc of the System name and the index in the System list.
    using SystemHandle = AZStd::tuple<AZ::Crc32, SystemIndex>;
    static constexpr SystemHandle InvalidSystemHandle = { AZ::Crc32(), static_cast<SystemIndex>(-1) };

    //! Ease of use type for referencing a List of SystemHandle objects.
    using SystemHandleList = AZStd::vector<SystemHandle>;

    //! A handle to a Simulated body within a physics system.
    //! A SimulatedBodyHandle is a tuple of a Crc of the system's name and the index in the SimulatedBody list.
    using SimulatedBodyHandle = AZStd::tuple<AZ::Crc32, SimulatedBodyIndex>;
    static constexpr SimulatedBodyHandle InvalidSimulatedBodyHandle = { AZ::Crc32(), -1 };
    using SimulatedBodyHandleList = AZStd::vector<SimulatedBodyHandle>;

    //! A handle to a Joint within a physics system.
    //! A JointHandle is a tuple of a Crc of the system's name and the index in the Joint list.
    using JointHandle = AZStd::tuple<AZ::Crc32, JointIndex>;
    static constexpr JointHandle InvalidJointHandle = { AZ::Crc32(), -1 };

    //! Helper used for pairing the ShapeConfiguration and ColliderConfiguration together which is used when creating a Simulated Body.
    using ShapeColliderPair = AZStd::pair<
        AZStd::shared_ptr<Physics::ColliderConfiguration>,
        AZStd::shared_ptr<Physics::ShapeConfiguration>>;
    using ShapeColliderPairList = AZStd::vector<ShapeColliderPair>;

        //! Joint types are used to request for AZ::TypeId with the JointHelpersInterface::GetSupportedJointTypeId.
    //! If the Physics backend supports this joint type JointHelpersInterface::GetSupportedJointTypeId will return a AZ::TypeId.
    enum class JointType
    {
        D6Joint,
        FixedJoint,
        BallJoint,
        HingeJoint
    };

    //! Flags used to specify which properties of a body to compute.
    enum class MassComputeFlags : AZ::u8
    {
        NONE = 0,

        //! Flags indicating whether a certain mass property should be auto-computed or not.
        COMPUTE_MASS = 1,
        COMPUTE_INERTIA = 1 << 1,
        COMPUTE_COM = 1 << 2,

        //! If set, non-simulated shapes will also be included in the mass properties calculation.
        INCLUDE_ALL_SHAPES = 1 << 3,

        DEFAULT = COMPUTE_COM | COMPUTE_INERTIA | COMPUTE_MASS
    };

    //! Bitwise operators for MassComputeFlags
    AZ_DEFINE_ENUM_BITWISE_OPERATORS(MassComputeFlags)

    //! Variant to allow support for the system to either create the Shape(s) or use the provide Shape(s) that have been created externally.
    //! Can be one of the following.
    //! @code{ .cpp }
    //! // A ShapeColliderPair, which contains a ColliderConfiguration and ShapeConfiguration.
    //! AzPhysics::StaticRigidBodyConfiguration staticRigidBodyConfig;
    //! staticRigidBodyConfig.m_colliderAndShapeData = AzPhysics::ShapeColliderPair(&colliderConfig, &shapeConfig);
    //!
    //! // A pointer to a Physics::Shape. The Simulated Body will take ownership of the pointer.
    //! AZStd::shared_ptr<Physics::Shape> shapePtr /*Created through other means*/;
    //! AzPhysics::StaticRigidBodyConfiguration staticRigidBodyConfig;
    //! staticRigidBodyConfig.m_colliderAndShapeData = shapePtr;
    //!
    //! // A list of ShapeColliderPairs.
    //! AZStd::vector<AzPhysics::ShapeColliderPair> shapeColliderPairList;
    //! shapeColliderPairList.emplace_back(&colliderConfig, &shapeConfig); //add as many configs as required.
    //! AzPhysics::StaticRigidBodyConfiguration staticRigidBodyConfig;
    //! staticRigidBodyConfig.m_colliderAndShapeData = shapeColliderPairList;
    //!
    //! // A list of Physics::Shape pointers. The Simulated Body will take ownership of these pointers.
    //! AZStd::vector<AZStd::shared_ptr<Physics::Shape>> shapePtrList;
    //! shapePtrList.emplace_back(/*Shape created through other means*/);
    //! AzPhysics::StaticRigidBodyConfiguration staticRigidBodyConfig;
    //! staticRigidBodyConfig.m_colliderAndShapeData = shapePtrList;
    //! @endcode
    using ShapeVariantData = AZStd::variant<
                                AZStd::monostate,
                                ShapeColliderPair,
                                AZStd::shared_ptr<Physics::Shape>,
                                AZStd::vector<ShapeColliderPair>,
                                AZStd::vector<AZStd::shared_ptr<Physics::Shape>>>;

    // Motion types matching Jolt but AZ-style
    enum class MotionType : uint8_t // TODO: Find the AzPhysics analogue
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

    // // Object layers (might want more sophisticated layer management later)
    // enum class ObjectLayer : uint16_t
    // {
    //     Static = 0,
    //     Moving = 1,
    //     Debris = 2,
    //     // Add more as needed
    // };
} // namespace JoltPhysics
