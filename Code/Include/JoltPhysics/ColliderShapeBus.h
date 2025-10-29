#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace JoltPhysics
{
    /// Messages serviced by a Jolt collider component's shape.
    /// A Jolt collider component shape represents the 3D geometry of a collider.
    class ColliderShapeRequests
        : public AZ::ComponentBus
    {
    public:
        /// Gets the world space AABB of the collider's shape.
        virtual AZ::Aabb GetColliderShapeAabb() = 0;

        /// Checks if this collider shape is a trigger.
        virtual bool IsTrigger() = 0;
    };
    using ColliderShapeRequestBus = AZ::EBus<ColliderShapeRequests>;
}
