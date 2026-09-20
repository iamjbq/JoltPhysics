
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzFramework/Physics/Shape.h>
#include <AzFramework/Physics/SimulatedBodies/RigidBody.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>

namespace AzPhysics
{
    class CollisionGroup;
    class CollisionLayer;
} // namespace AzPhysics

// namespace JPH
// {
//     class Ref;
//     class ConvexHullShape;
//     class MeshShape;
//     class HeightFieldShape;
// }

namespace JoltPhysics
{
    /// Requests for the Jolt system component.
    /// The system component owns fundamental Jolt objects which manage worlds, rigid bodies, shapes, materials,
    /// constraints etc., and perform cooking (processing assets such as meshes and heightfields ready for use in Jolt).
    class SystemRequests
        : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

        using MutexType = AZStd::recursive_mutex;

        virtual ~SystemRequests() = default;

        /// Creates a new convex hull.
        /// @param vertices Pointer to beginning of vertex data.
        /// @param vertexNum Number of vertices in mesh.
        /// @param vertexStride Size of each entry in the vertex data.
        /// @return Pointer to the created mesh.
        virtual JPH::Ref<JPH::ConvexHullShape> CreateConvexHull(const void* vertices, AZ::u32 vertexNum, AZ::u32 vertexStride) = 0; // should we use AZ::Vector3* or JPH::Vec3 here?

        /// Creates a new convex mesh from pre-cooked convex mesh data.
        /// @param cookedMeshData Pointer to the cooked convex mesh data.
        /// @param bufferSize Size of the cookedMeshData buffer in bytes.
        /// @return Pointer to the created convex mesh.
        virtual JPH::Ref<JPH::ConvexHullShape> CreateConvexHullFromCooked(const void* cookedMeshData, AZ::u32 bufferSize) = 0;

        /// Creates a new triangle mesh from pre-cooked mesh data.
        /// @param cookedMeshData Pointer to the cooked mesh data.
        /// @param bufferSize Size of the cookedMeshData buffer in bytes.
        /// @return Pointer to the created mesh.
        virtual JPH::Ref<JPH::MeshShape> CreateTriangleMeshFromCooked(const void* cookedMeshData, AZ::u32 bufferSize) = 0;

        /// Creates a new heightfield.
        /// @param samples Pointer to beginning of heightfield sample data.
        /// @param numColumns Number of columns in the heightfield.
        /// @param numRows Number of rows in the heightfield.
        /// @return Pointer to the created heightfield.
        virtual JPH::Ref<JPH::HeightFieldShape> CreateHeightField(const float* samples, size_t numColumns, size_t numRows) = 0;

        /// Creates PhysX collision filter data from generic collision filtering settings.
        /// @param layer The collision layer the object belongs to.
        /// @param group The set of collision layers the object will interact with.
        // virtual physx::PxFilterData CreateFilterData(const AzPhysics::CollisionLayer& layer, const AzPhysics::CollisionGroup& group) = 0;
    };

    using SystemRequestsBus = AZ::EBus<SystemRequests>;
} // namespace JoltPhysics
