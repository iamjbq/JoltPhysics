#pragma once

#include<AzFramework/Physics/Common/PhysicsSceneQueries.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/CastResult.h>

namespace JPH
{
    class PhysicsSystem;
}

namespace JoltPhysics
{
    class Shape;

    namespace SceneQueryHelpers
    {
        //! Helper function to convert from Jolt hit to AZ.
        AzPhysics::SceneQueryHit GetHitFromJoltHit(const JPH::RayCastResult& joltHit, const JPH::PhysicsSystem& sceneSystem);
    }
}
