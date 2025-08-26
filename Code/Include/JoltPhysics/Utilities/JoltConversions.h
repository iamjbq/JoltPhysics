
#pragma once

#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Vector4.h>
#include <AzCore/Math/Quaternion.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Math/Matrix3x3.h>

#include <JoltPhysics/JoltPhysicsTypes.h>

// Forward declare Jolt types to avoid including Jolt headers in public interface
namespace JPH
{
    class Vec3;
    class Vec4;
    class Quat;
    class Mat44;
    enum class EMotionType;
    enum class EActivation;
}

namespace JoltPhysics::Conversions
{
    // Vector conversions
    AZ::Vector3 JoltToAzVector3(const JPH::Vec3& joltVec);
    JPH::Vec3 AzToJoltVec3(const AZ::Vector3& azVec);

    // Vector4 conversions
    AZ::Vector4 JoltToAzVector4(const JPH::Vec4& joltVec);
    JPH::Vec4 AzToJoltVec4(const AZ::Vector4& azVec);

    // Quaternion conversions
    AZ::Quaternion JoltToAzQuaternion(const JPH::Quat& joltQuat);
    JPH::Quat AzToJoltQuat(const AZ::Quaternion& azQuat);

    // Transform/Matrix conversions
    AZ::Transform JoltToAzTransform(const JPH::Mat44& joltMatrix);
    JPH::Mat44 AzToJoltMat44(const AZ::Transform& azTransform);

    // Enum conversions
    MotionType JoltToAzMotionType(JPH::EMotionType joltMotionType);
    JPH::EMotionType AzToJoltMotionType(MotionType azMotionType);

    ActivationState JoltToAzActivationState(JPH::EActivation joltActivation);
    JPH::EActivation AzToJoltActivation(ActivationState azActivation);

} // namespace JoltPhysics::Conversions
