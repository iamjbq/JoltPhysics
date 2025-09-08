
#pragma once

#include <AzCore/Math/Quaternion.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Aabb.h>
#include <AzCore/Math/Transform.h>

#include <Jolt/Jolt.h>
// #include <Jolt/Math/Vec3.h>

inline JPH::Vec3 JoltMathConvert(const AZ::Vector3& inVec)
{
    return JPH::Vec3(inVec.GetX(), inVec.GetY(), inVec.GetZ());
}

inline AZ::Vector3 JoltMathConvert(const JPH::Vec3& inVec)
{
    return AZ::Vector3(inVec.GetX(), inVec.GetY(), inVec.GetZ());
}

inline AZ::Vector4 JoltMathConvert(const JPH::Vec4& inVec)
{
    return AZ::Vector4(inVec.GetX(), inVec.GetY(), inVec.GetZ(), inVec.GetW());
}

inline JPH::Vec4 JoltMathConvert(const AZ::Vector4& inVec)
{
    return JPH::Vec4(inVec.GetX(), inVec.GetY(), inVec.GetZ(), inVec.GetW());
}

inline JPH::Quat JoltMathConvert(const AZ::Quaternion& inQuat)
{
    return JPH::Quat(inQuat.GetX(), inQuat.GetY(), inQuat.GetZ(), inQuat.GetW());
}

inline AZ::Quaternion JoltMathConvert(const JPH::Quat& inQuat)
{
    return AZ::Quaternion(inQuat.GetX(), inQuat.GetY(), inQuat.GetZ(), inQuat.GetW());
}

// Why is there only Jolt->AZ direction for this one?
inline AZ::Aabb JoltMathConvert(const JPH::AABox& bounds)
{
    // check that the Jolt bounds are valid, otherwise CreateFromMinMax will assert.
    if (bounds.IsValid())
    {
        return AZ::Aabb::CreateFromMinMax(JoltMathConvert(bounds.mMin), JoltMathConvert(bounds.mMax));
    }
    return AZ::Aabb::CreateNull();
}

// inline JPH::Mat44 JoltMathConvert(const AZ::Transform& lyTransform)
// {
//     AZ::Quaternion quat = lyTransform.GetRotation();
//     AZ::Vector3 vec3 = lyTransform.GetTranslation();
//
//     return JPH::Mat44(JoltMathConvert(vec3),
//         JoltMathConvert(quat).getNormalized());
// }
//
// inline AZ::Transform JoltMathConvert(const physx::PxTransform& pxTransform)
// {
//     return AZ::Transform::CreateFromQuaternionAndTranslation(JoltMathConvert(pxTransform.q), JoltMathConvert(pxTransform.p));
// }
