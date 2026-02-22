
#pragma once

#include <Clients/BaseColliderComponent.h>
#include <AzCore/Component/Component.h>
#include <AzFramework/Physics/ShapeConfiguration.h>

namespace JoltPhysics
{
    //! Component that provides box shape collider.
    //! May be used in conjunction with a Jolt Rigid Body Component to create a dynamic rigid body, or on its own
    //! to create a static rigid body. // TODO: Make this require a static rigid body explicity?
    class BoxColliderComponent
        : public BaseColliderComponent
    {
    public:
        using Configuration = Physics::BoxShapeConfiguration;
        AZ_COMPONENT(BoxColliderComponent, "{9A97967C-0ED2-4ED2-B31E-EE282454A355}", BaseColliderComponent);
        static void Reflect(AZ::ReflectContext* context);

        BoxColliderComponent() = default;

        // BaseColliderComponent
        void UpdateScaleForShapeConfigs() override;
    };
} // JoltPhysics
