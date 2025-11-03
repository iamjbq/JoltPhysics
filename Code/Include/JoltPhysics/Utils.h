
#pragma once

#include <JoltPhysics/BodyData.h>

#include <Jolt/Physics/Collision/Shape/Shape.h>

namespace AzPhysics
{
    class CollisionLayer;
    class CollisionGroup;
    class Scene;
}

namespace Physics
{
    class Shape;
}

namespace JPH
{
    class Body;
    class Shape;
}

namespace JoltPhysics
{
    enum class CombineMode : AZ::u8;
    class JoltPhysicsMaterial;
    
    namespace Utils
    {
        BodyData* GetUserData(const JPH::Body& body);
        Physics::Material* GetUserData(const JoltPhysicsMaterial* material);
        Physics::Shape* GetUserData(const JPH::Shape* shape);

        //! Returns the effective friction or restitution for two materials in contact.
        //! inMode should be the highest value of CombineMode enum for the pair.
        float GetCombinedMaterialProperty(const float& inValue1, const float& inValue2, const JoltPhysics::CombineMode& inMode);
    }
}

#include <JoltPhysics/Utils.inl>
