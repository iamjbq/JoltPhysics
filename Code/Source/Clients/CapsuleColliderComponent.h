
#pragma once

#include <Clients/BaseColliderComponent.h>
#include <AzCore/Component/Component.h>

namespace JoltPhysics
{
    /// Component that provides capsule shape collider.
    /// May be used in conjunction with a Jolt Rigid Body Component to create a dynamic rigid body, or on its own
    /// to create a static rigid body.
    class CapsuleColliderComponent
        : public BaseColliderComponent
    {
    public:
        using Configuration = Physics::CapsuleShapeConfiguration;
        AZ_COMPONENT(CapsuleColliderComponent, "{00247433-1F59-442B-9F5C-782D22CB0194}", BaseColliderComponent);
        static void Reflect(AZ::ReflectContext* context);

        CapsuleColliderComponent() = default;

        // BaseColliderComponent
        void UpdateScaleForShapeConfigs() override;
    };
} // JoltPhysics
