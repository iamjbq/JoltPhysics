
#include "Material/JoltMaterial.h"
#include "Material/JoltPhysicsMaterial.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

namespace JoltPhysics
{
    inline BodyData* Utils::GetUserData(const JPH::Body& body)
    {
        if (body.GetUserData() == 0)
        {
            return nullptr;
        }

        auto bodyData = reinterpret_cast<BodyData*>(body.GetUserData());
        if (!bodyData || !bodyData->IsValid())
        {
            AZ_Warning("JoltPhysics::Utils::GetUserData", false, "The body data does not look valid and is not safe to use");
            return nullptr;
        }

        return bodyData;
    }

    inline Physics::Material* Utils::GetUserData(const JoltPhysics::JoltPhysicsMaterial* material)
    {
        return (material == nullptr) ? nullptr : static_cast<Physics::Material*>(material->m_userData);
    }

    inline Physics::Shape* Utils::GetUserData(const JPH::Shape* shape)
    {
        return (shape == nullptr) ? nullptr : reinterpret_cast<Physics::Shape*>(shape->GetUserData());
    }

    inline float Utils::GetCombinedMaterialProperty(const float& inValue1, const float& inValue2, const CombineMode& inMode)
    {
        switch (inMode)
        {
            case CombineMode::Average:
                return AZ::Sqrt(inValue1 * inValue2); // Jolt uses the geometric mean
            case CombineMode::Minimum:
                return AZStd::min(inValue1, inValue2);
            case CombineMode::Maximum:
                return AZStd::max(inValue1, inValue2);
            case CombineMode::Multiply:
                return inValue1 * inValue2;
            default:
                return AZ::Sqrt(inValue1 * inValue2);
        }
    }
}
