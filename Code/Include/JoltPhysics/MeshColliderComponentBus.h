
#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <JoltPhysics/MeshAsset.h>

namespace Physics
{
    struct MaterialId;
}

namespace JoltPhysics
{
    /// Services provided by the Jolt Mesh Collider Component.
    class MeshColliderComponentRequests
        : public AZ::ComponentBus
    {
    public:
        /// Gets Mesh information from this entity.
        /// @return Asset pointer to mesh asset.
        virtual AZ::Data::Asset<Pipeline::MeshAsset> GetMeshAsset() const = 0;

        /// Sets the mesh asset ID
        /// @note Calling this function will not recreate the physics shapes of the collider.
        /// @param id The asset ID to set it to.
        virtual void SetMeshAsset(const AZ::Data::AssetId& id) = 0;
    };

    /// Bus to service the Jolt Mesh Collider Component event group.
    using MeshColliderComponentRequestsBus = AZ::EBus<MeshColliderComponentRequests>;
} // namespace JoltPhysics
