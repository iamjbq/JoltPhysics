#pragma once

#include <AzCore/Component/Entity.h>
#include <AzCore/Math/Quaternion.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Math/Vector3.h>
#include <AzFramework/Physics/Material/PhysicsMaterialSlots.h>
#include <AzFramework/Physics/Shape.h>
#include <AzFramework/Physics/ShapeConfiguration.h>
#include <AzCore/std/optional.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

namespace AzPhysics
{
    class CollisionGroup;
    struct RigidBodyConfiguration;
    struct StaticRigidBodyConfiguration;
    struct StaticRigidBody;
    class Scene;
}

namespace Physics
{
    class RigidBodyConfiguration;
    class ColliderConfiguration;
    class ShapeConfiguration;
}

namespace JoltPhysics
{
    class Shape;
    class BodyData;

    namespace Pipeline
    {
        class MeshAssetData;
    } // namespace Pipeline

    namespace Utils
    {
        bool ComputeJoltShapeFromConfig(const Physics::ShapeConfiguration& shapeConfiguration, JPH::Shape* outShape);

        void CreateJoltShapeResultFromHeightField(Physics::HeightfieldShapeConfiguration& heightfieldConfig, JPH::Shape* outShape);

        AZStd::vector<float> ConvertHeightfieldSamples(
            const Physics::HeightfieldShapeConfiguration& heightfield,
            const size_t startCol, const size_t startRow,
            const size_t numColsToUpdate, const size_t numRowsToUpdate);

        JPH::Shape* CreateJoltShapeFromConfig(
            const Physics::ColliderConfiguration& colliderConfiguration,
            const Physics::ShapeConfiguration& shapeConfiguration,
            AzPhysics::CollisionGroup& assignedCollisionGroup
        );
    } // namespace Utils
}
