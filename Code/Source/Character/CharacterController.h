
#pragma once

#include <AzFramework/Physics/Character.h>
#include <JoltPhysics/BodyData.h>

#include <Jolt/Jolt.h>
// #include <Jolt/Physics/Character/CharacterVirtual.h>

#include <AzFramework/Physics/Collision/CollisionGroups.h>
#include <AzFramework/Physics/Collision/CollisionLayers.h>
#include <AzFramework/Physics/Common/PhysicsTypes.h>

#include "Jolt/Physics/Character/CharacterVirtual.h"

namespace JoltPhysics
{
    class JoltCharacterContactListener 
        : public JPH::CharacterContactListener
    {
    public:
        void OnContactAdded(const JPH::CharacterVirtual* inCharacter, const JPH::BodyID& inBodyID2, const JPH::SubShapeID& inSubShapeID2, JPH::RVec3Arg inContactPosition, JPH::Vec3Arg inContactNormal, JPH::CharacterContactSettings& ioSettings) override;
        void OnCharacterContactAdded(const JPH::CharacterVirtual* inCharacter, const JPH::CharacterVirtual* inOtherCharacter, const JPH::SubShapeID& inSubShapeID2, JPH::RVec3Arg inContactPosition, JPH::Vec3Arg inContactNormal, JPH::CharacterContactSettings& ioSettings) override;
        void OnAdjustBodyVelocity(const JPH::CharacterVirtual* inCharacter, const JPH::Body& inBody2, JPH::Vec3& ioLinearVelocity, JPH::Vec3& ioAngularVelocity) override;
    };
    
    class CharacterController
        : Physics::Character
    {
    public:
        AZ_CLASS_ALLOCATOR(CharacterController, AZ::SystemAllocator);
        AZ_RTTI(JoltPhysics::CharacterController, "{2C8F1645-8FD5-4055-91B7-3BFAA9C77E16}", Physics::Character);
        static void Reflect(AZ::ReflectContext* context);

        CharacterController() = default;
        CharacterController(AzPhysics::SceneHandle sceneHandle);
        ~CharacterController();
        
        void EnablePhysics(const Physics::CharacterConfiguration& configuration);
        void DisablePhysics();

        // Physics::Character
        AZ::Vector3 GetBasePosition() const override;
        void SetBasePosition(const AZ::Vector3& position) override;
        AZ::Vector3 GetCenterPosition() const override;
        float GetStepHeight() const override;
        void SetStepHeight(float stepHeight) override;
        AZ::Vector3 GetUpDirection() const override;
        void SetUpDirection(const AZ::Vector3& upDirection) override;
        float GetSlopeLimitDegrees() const override;
        void SetSlopeLimitDegrees(float slopeLimitDegrees) override;
        float GetMaximumSpeed() const override;
        void SetMaximumSpeed(float maximumSpeed) override;
        AZ::Vector3 GetVelocity() const override;
        AzPhysics::CollisionLayer GetCollisionLayer() const override;
        AzPhysics::CollisionGroup GetCollisionGroup() const override;
        void SetCollisionLayer(const AzPhysics::CollisionLayer& layer) override;
        void SetCollisionGroup(const AzPhysics::CollisionGroup& group) override;
        AZ::Crc32 GetColliderTag() const override;
        void AddVelocityForTick(const AZ::Vector3& velocity) override;
        void AddVelocityForPhysicsTimestep(const AZ::Vector3& velocity) override;
        void ResetRequestedVelocityForTick() override;
        void ResetRequestedVelocityForPhysicsTimestep() override;
        void Move(const AZ::Vector3& requestedMovement, float deltaTime) override;
        void ApplyRequestedVelocity(float deltaTime) override;
        void SetRotation(const AZ::Quaternion& rotation) override;
        void AttachShape(AZStd::shared_ptr<Physics::Shape> shape) override;

        // AzPhysics::SimulatedBody
        AZ::EntityId GetEntityId() const override;
        AzPhysics::Scene* GetScene() override;
        AZ::Transform GetTransform() const override;
        void SetTransform(const AZ::Transform& transform) override;
        AZ::Vector3 GetPosition() const override;
        AZ::Quaternion GetOrientation() const override;
        AZ::Aabb GetAabb() const override;
        AzPhysics::SceneQueryHit RayCast(const AzPhysics::RayCastRequest& request) override;
        AZ::Crc32 GetNativeType() const override;
        void* GetNativePointer() const override;

        // CharacterController specific
        void Resize(float height);
        float GetHeight() const;
        void SetHeight(float height);
        float GetRadius() const;
        void SetRadius(float radius);
        float GetHalfSideExtent() const;
        void SetHalfSideExtent(float halfSideExtent);
        float GetHalfForwardExtent() const;
        void SetHalfForwardExtent(float halfForwardExtent);

    private:
        void SetFilterDataAndShape(const Physics::CharacterConfiguration& characterConfig);
        void SetUserData(const Physics::CharacterConfiguration& characterConfig);
        void SetActorName(const AZStd::string& name = "Character Controller");
        void SetMinimumMovementDistance(float distance);
        void SetTag(const AZStd::string& tag);
        void CreateShadowBody(const Physics::CharacterConfiguration& configuration);
        void DestroyShadowBody();
        void RemoveControllerFromScene();
        void UpdateFilterLayerAndGroup(AzPhysics::CollisionLayer collisionLayer, AzPhysics::CollisionGroup collisionGroup);

        // physx::PxController* m_pxController = nullptr; ///< The underlying PhysX controller.
        float m_minimumMovementDistance = 0.0f; ///< To avoid jittering, the controller will not attempt to move distances below this.
        AZ::Vector3 m_requestedVelocityForTick = AZ::Vector3::CreateZero(); ///< Used to accumulate velocity requests which last for a tick.
        AZ::Vector3 m_requestedVelocityForPhysicsTimestep =
            AZ::Vector3::CreateZero(); ///< Used to accumulate velocity requests which last for a physics timestep.
        AZ::Vector3 m_observedVelocity = AZ::Vector3::CreateZero(); ///< Velocity observed in the simulation, may not match desired.
        JoltPhysics::BodyData m_bodyUserData; ///< Used to populate the user data associated with the controller.
        // physx::PxFilterData m_filterData; ///< Controls filtering for collisions with other objects and scene queries.
        // physx::PxControllerFilters m_pxControllerFilters; ///< Controls which objects the controller interacts with when moving.
        AZStd::shared_ptr<Physics::Shape> m_shape; ///< The generic physics API shape associated with the controller.
        AzPhysics::RigidBody* m_shadowBody = nullptr; ///< A kinematic-synchronized rigid body used to store additional colliders.
        AzPhysics::SimulatedBodyHandle m_shadowBodyHandle = AzPhysics::InvalidSimulatedBodyHandle; //!<A handle to the shadow body.
        AZStd::string m_name = "Character Controller"; ///< Name to set on the body associated with the controller.
        AZ::Crc32 m_colliderTag; ///< Tag used to identify the collider associated with the controller.
        float m_maximumSpeed = 100.0f; ///< If the accumulated requested velocity for a tick exceeds this magnitude, it will be clamped.
        // AZStd::unique_ptr<CharacterControllerCallbackManager>
        //     m_callbackManager; ///< Manages callbacks for collision filtering, collision notifications, and handling riding on objects.
        AZStd::unique_ptr<JPH::CharacterContactListener> m_contactListener;
        AZ::Quaternion m_orientation; ///< The orientation of the character.
    };

    //! Example implementation of controller-controller filtering callback.
    //! This example causes controllers to impede each other's movement based on their collision filters.
    // bool CollisionLayerBasedControllerFilter(const physx::PxController& controllerA, const physx::PxController& controllerB);

    //! Example implementation of controller-object filtering callback.
    //! This example causes static and kinematic bodies to impede the character based on collision layers.
    // physx::PxQueryHitType::Enum CollisionLayerBasedObjectPreFilter(
    //     const physx::PxFilterData& filterData,
    //     const physx::PxShape* shape,
    //     const physx::PxRigidActor* actor,
    //     [[maybe_unused]] physx::PxHitFlags& queryFlags);
    // };
}


