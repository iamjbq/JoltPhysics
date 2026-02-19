
#pragma once

#include <Clients/BaseColliderComponent.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Math/Vector3.h>

namespace JoltPhysics
{
    /// Component that provides sphere shape collider.
    /// May be used in conjunction with a Jolt Rigid Body Component to create a dynamic rigid body, or on its own
    /// to create a static rigid body.
    class SphereColliderComponent
        : public BaseColliderComponent
    {
    public:
        using Configuration = Physics::SphereShapeConfiguration;
        AZ_COMPONENT(SphereColliderComponent, "{FCAA203C-D9FE-43D6-B2BC-2F22478551BA}", BaseColliderComponent);
        static void Reflect(AZ::ReflectContext* context);

        SphereColliderComponent() = default;

        // BaseColliderComponent
        void UpdateScaleForShapeConfigs() override;
    };
} // JoltPhysics
