#pragma once

#include <AzCore/Component/Entity.h>
#include <AzCore/Math/Quaternion.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Math/Vector3.h>
#include <AzFramework/Physics/Material/PhysicsMaterialSlots.h>
#include <AzFramework/Physics/Shape.h>
#include <AzFramework/Physics/ShapeConfiguration.h>
#include <AzCore/std/optional.h>

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

namespace JPH
{
    class Shape;
    class BroadPhaseLayer;
}

namespace JoltPhysics
{
    class BodyData;
    class JoltPhysicsMaterial;

    namespace Pipeline
    {
        class MeshAssetData;
    } // namespace Pipeline

    namespace Utils
    {
        AzPhysics::Scene* GetDefaultScene();

        bool ComputeJoltShapeFromConfig(
            const Physics::ShapeConfiguration& shapeConfiguration,
            JPH::Shape::ShapeResult& outResult,
            AZStd::vector<const JoltPhysicsMaterial*>& inMaterials);
        
        void CreateJoltShapeResultFromHeightField(
            Physics::HeightfieldShapeConfiguration& heightfieldConfig,
            JPH::Shape::ShapeResult& outResult,
            AZStd::vector<const JoltPhysicsMaterial*>& inMaterials);

        AZStd::vector<float> ConvertHeightfieldSamples(
            const Physics::HeightfieldShapeConfiguration& heightfield,
            const size_t startCol, const size_t startRow,
            const size_t numColsToUpdate, const size_t numRowsToUpdate);

        JPH::Shape* CreateJoltShapeFromConfig(
            const Physics::ColliderConfiguration& colliderConfiguration,
            const Physics::ShapeConfiguration& shapeConfiguration,
            AzPhysics::CollisionGroup& assignedCollisionGroup
        );

        // JPH::ObjectLayer ConstructObjectLayer(
        //     const Physics::ColliderConfiguration& colliderConfiguration,
        //     const AzPhysics::CollisionGroup& assignedCollisionGroup,
        //     const JPH::BroadPhaseLayer& broadPhaseLayer);

        //! Converts collider position and orientation offsets to a transform.
        AZ::Transform GetColliderLocalTransform(const AZ::Vector3& colliderRelativePosition
            , const AZ::Quaternion& colliderRelativeRotation);

        //! Gets the local transform for a collider (the position and rotation relative to its entity).
        AZ::Transform GetColliderLocalTransform(const AZ::EntityComponentIdPair& idPair);

        //! Combines collider position and orientation offsets and world transform to a transform.
        AZ::Transform GetColliderWorldTransform(const AZ::Transform& worldTransform
            , const AZ::Vector3& colliderRelativePosition
            , const AZ::Quaternion& colliderRelativeRotation);

        //! Converts points in a collider's local space to world space positions
        //! accounting for collider position and orientation offsets.
        void ColliderPointsLocalToWorld(AZStd::vector<AZ::Vector3>& pointsInOut
            , const AZ::Transform& worldTransform
            , const AZ::Vector3& colliderRelativePosition
            , const AZ::Quaternion& colliderRelativeRotation
            , const AZ::Vector3& nonUniformScale);

        //! Returns AABB of collider by constructing PxGeometry from collider and shape configuration,
        //! and invoking physx::PxGeometryQuery::getWorldBounds.
        //! This function is used only by editor components.
        AZ::Aabb GetColliderAabb(const AZ::Transform& worldTransform
            , bool hasNonUniformScale
            , AZ::u8 subdivisionLevel
            , const ::Physics::ShapeConfiguration& shapeConfiguration
            , const ::Physics::ColliderConfiguration& colliderConfiguration);

        bool TriggerColliderExists(AZ::EntityId entityId);

        void CreateShapesFromAsset(const Physics::PhysicsAssetShapeConfiguration& assetConfiguration,
            const Physics::ColliderConfiguration& originalColliderConfiguration, bool hasNonUniformScale,
            AZ::u8 subdivisionLevel, AZStd::vector<AZStd::shared_ptr<Physics::Shape>>& resultingShapes);

        void GetColliderShapeConfigsFromAsset(const Physics::PhysicsAssetShapeConfiguration& assetConfiguration,
            const Physics::ColliderConfiguration& originalColliderConfiguration,
            bool hasNonUniformScale, AZ::u8 subdivisionLevel, AzPhysics::ShapeColliderPairList& resultingColliderShapes);

        //! Gets the scale from the entity's Transform component.
        float GetTransformScale(AZ::EntityId entityId);
        //! Returns a vector scale with each element equal to the max element from the entity's Transform component.
        AZ::Vector3 GetUniformScale(AZ::EntityId entityId);
        //! Gets the scale from the entity's Non-Uniform Scale component, if it is present.
        //! Otherwise (1, 1, 1) is returned.
        AZ::Vector3 GetNonUniformScale(AZ::EntityId entityId);
        //! Gets the overall scale, taking into account the scale from both the entity's Transform component and the
        //! Non-Uniform Scale component, if it is present.
        AZ::Vector3 GetOverallScale(AZ::EntityId entityId);
    } // namespace Utils
}
