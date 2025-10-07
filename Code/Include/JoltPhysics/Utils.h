
#pragma once

#include <JoltPhysics/BodyData.h>

#include "Jolt/Physics/Body/Body.h"

namespace AzPhysics
{
    class CollisionLayer;
    class CollisionGroup;
    class Scene;
}

namespace Physics
{
    class Material;
    class Shape;
}

namespace JoltPhysics
{
    namespace Utils
    {
        BodyData* GetUserData(const JPH::Body& body);
    }
}

#include <JoltPhysics/Utils.inl>
