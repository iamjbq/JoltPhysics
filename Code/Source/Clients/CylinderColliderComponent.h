
#pragma once

#include <Clients/BaseColliderComponent.h>
#include <AzCore/Component/Component.h>

namespace JoltPhysics
{
    /// Component that provides cylinder shape collider.
    /// May be used in conjunction with a Jolt Rigid Body Component to create a dynamic rigid body, or on its own
    /// to create a static rigid body.
    class CylinderColliderComponent
        : public BaseColliderComponent
    {
    public:
        using Configuration = Physics::CapsuleShapeConfiguration;
        AZ_COMPONENT(CylinderColliderComponent, "{756204DD-8A77-480A-ACE6-45294C8ADCC9}", BaseColliderComponent);
        static void Reflect(AZ::ReflectContext* context);

        CylinderColliderComponent() = default;

        // BaseColliderComponent
        void UpdateScaleForShapeConfigs() override;
    };
} // JoltPhysics
