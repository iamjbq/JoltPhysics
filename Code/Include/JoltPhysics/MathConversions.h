
#pragma once

#include <AzCore/Math/Quaternion.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Aabb.h>
#include <AzCore/Math/Transform.h>

#include <Jolt/Jolt.h>
#include <Jolt/Math/Vec3.h>
#include <Jolt/Geometry/AABox.h>

AZ_FORCE_INLINE JPH::Vec3 JoltMathConvert(const AZ::Vector3& inVec)
{
    return JPH::Vec3(inVec.GetX(), inVec.GetY(), inVec.GetZ());
}

AZ_FORCE_INLINE AZ::Vector3 JoltMathConvert(const JPH::Vec3& inVec)
{
    return AZ::Vector3(inVec.GetX(), inVec.GetY(), inVec.GetZ());
}

AZ_FORCE_INLINE AZ::Vector4 JoltMathConvert(const JPH::Vec4& inVec)
{
    return AZ::Vector4(inVec.GetX(), inVec.GetY(), inVec.GetZ(), inVec.GetW());
}

AZ_FORCE_INLINE JPH::Vec4 JoltMathConvert(const AZ::Vector4& inVec)
{
    return JPH::Vec4(inVec.GetX(), inVec.GetY(), inVec.GetZ(), inVec.GetW());
}

AZ_FORCE_INLINE JPH::Quat JoltMathConvert(const AZ::Quaternion& inQuat)
{
    return JPH::Quat(inQuat.GetX(), inQuat.GetY(), inQuat.GetZ(), inQuat.GetW());
}

AZ_FORCE_INLINE AZ::Quaternion JoltMathConvert(const JPH::Quat& inQuat)
{
    return AZ::Quaternion(inQuat.GetX(), inQuat.GetY(), inQuat.GetZ(), inQuat.GetW());
}

// Note that a JPH::Mat44 must convert to 3x3, usually from Inversed3x3()
AZ_FORCE_INLINE AZ::Matrix3x3 JoltMathConvert(const JPH::Mat44& inMat)
{
    return AZ::Matrix3x3::CreateFromRows(
        JoltMathConvert(inMat.GetAxisX()),
        JoltMathConvert(inMat.GetAxisY()),
        JoltMathConvert(inMat.GetAxisZ())
        );
}

// Why is there only Jolt->AZ direction for this one?
AZ_FORCE_INLINE AZ::Aabb JoltMathConvert(const JPH::AABox& bounds)
{
    // check that the Jolt bounds are valid, otherwise CreateFromMinMax will assert.
    if (bounds.IsValid())
    {
        return AZ::Aabb::CreateFromMinMax(JoltMathConvert(bounds.mMin), JoltMathConvert(bounds.mMax));
    }
    return AZ::Aabb::CreateNull();
}

AZ_FORCE_INLINE AZ::Transform JoltMathConvert(const JPH::Vec3& position, const JPH::Quat& rotation)
{
    return AZ::Transform::CreateFromQuaternionAndTranslation(JoltMathConvert(rotation), JoltMathConvert(position));
}
