#pragma once

#include <Clients/BaseColliderComponent.h>

namespace LmbrCentral
{
    class CapsuleShapeConfig;
} // namespace LmbrCentral

/// This is the runtime component for the Shape Collider Component. It takes the shape config from a Shape component
/// These are part of the LumbrCentral gem, and simplify the work.

namespace JoltPhysics
{
    namespace Utils
    {
        Physics::CapsuleShapeConfiguration ConvertFromLmbrCentralCapsuleConfig(
            const LmbrCentral::CapsuleShapeConfig& inputCapsuleConfig, const AZ::Vector3& scale = Physics::ShapeConstants::DefaultScale);
    } // namespace Utils

    namespace ShapeConstants
    {
        static const AZ::Crc32 Box = AZ_CRC_CE("Box");
        static const AZ::Crc32 Capsule = AZ_CRC_CE("Capsule");
        static const AZ::Crc32 Sphere = AZ_CRC_CE("Sphere");
        static const AZ::Crc32 PolygonPrism = AZ_CRC_CE("PolygonPrism");
        static const AZ::Crc32 Cylinder = AZ_CRC_CE("Cylinder");
        static const AZ::Crc32 Quad = AZ_CRC_CE("QuadShape");
    } // namespace ShapeConstants

    /// Component that provides a collider based on geometry from a shape component.
    class ShapeColliderComponent
        : public BaseColliderComponent
    {
    public:
        AZ_COMPONENT(ShapeColliderComponent, "{30CC9E77-378C-49DF-9617-6BF191901FE0}", BaseColliderComponent);
        static void Reflect(AZ::ReflectContext* context);

        ShapeColliderComponent() = default;

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        // BaseColliderComponent
        void UpdateScaleForShapeConfigs() override;
    };
}
