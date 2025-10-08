
#pragma once

#include <JoltPhysics/BodyData.h>

#include "Jolt/Physics/Body/Body.h"
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

namespace JoltPhysics
{
    namespace Utils
    {
        BodyData* GetUserData(const JPH::Body& body);
        Physics::Shape* GetUserData(const JPH::Shape* shape);
    }
}

#include <JoltPhysics/Utils.inl>
