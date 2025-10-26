#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Component/NonUniformScaleBus.h>
#include <AzCore/Casting/lossy_cast.h>
#include <AzCore/EBus/Results.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/Utils.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/SimdMath.h>
#include <AzCore/Math/MathStringConversions.h>
#include <AzFramework/Physics/ShapeConfiguration.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Collision/CollisionGroups.h>
#include <AzFramework/Physics/Collision/CollisionLayers.h>
#include <AzFramework/Physics/Configuration/RigidBodyConfiguration.h>
#include <AzFramework/Physics/Configuration/StaticRigidBodyConfiguration.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Physics/SimulatedBodies/StaticRigidBody.h>
#include <AzFramework/Physics/HeightfieldProviderBus.h>

#include <JoltPhysics/Utils.h>
#include "Utils.h"
#include <JoltPhysics/Material/JoltMaterialConfiguration.h>
#include <JoltPhysics/MathConversions.h>
#include <Clients/JoltPhysicsSystemComponent.h>
#include <Clients/Shape.h>

#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/HeightFieldShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/PlaneShape.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"

namespace JoltPhysics
{
    namespace Utils
    {
        bool ComputeJoltShapeFromConfig(const Physics::ShapeConfiguration& shapeConfiguration, JPH::Shape* outShape)
        {
            if (!shapeConfiguration.m_scale.IsGreaterThan(AZ::Vector3::CreateZero()))
            {
                AZ_Error("Jolt Utils", false, "Negative or zero values are invalid for shape configuration scale values %s",
                    AZStd::to_string(shapeConfiguration.m_scale).c_str())
                return false;
            }

            switch (auto shapeType = shapeConfiguration.GetShapeType())
            {
            case Physics::ShapeType::Sphere:
                {
                    const auto& sphereConfig = dynamic_cast<const Physics::SphereShapeConfiguration&>(shapeConfiguration);
                    if (sphereConfig.m_radius <= 0.0f)
                    {
                        AZ_Error("Jolt Utils", false, "Invalid radius value: %f", sphereConfig.m_radius)
                        return false;
                    }
                    
                    outShape = new JPH::SphereShape(sphereConfig.m_radius * shapeConfiguration.m_scale.GetMaxElement());
                    break;
                }
            case Physics::ShapeType::Box:
                {
                    const auto& boxConfig = dynamic_cast<const Physics::BoxShapeConfiguration&>(shapeConfiguration);
                    if (!boxConfig.m_dimensions.IsGreaterThan(AZ::Vector3::CreateZero()))
                    {
                        AZ_Error("Jolt Utils", false, "Negative or zero values are invalid for box dimensions %s",
                            AZStd::to_string(boxConfig.m_dimensions).c_str())
                        return false;
                    }
                    
                    outShape = new JPH::BoxShape(JoltMathConvert(boxConfig.m_dimensions * 0.5f * shapeConfiguration.m_scale));
                    break;
                }
            case Physics::ShapeType::Capsule:
                {
                    const auto& capsuleConfig = dynamic_cast<const Physics::CapsuleShapeConfiguration&>(shapeConfiguration);
                    float height = capsuleConfig.m_height * capsuleConfig.m_scale.GetZ();
                    float radius = capsuleConfig.m_radius * AZ::GetMax(capsuleConfig.m_scale.GetX(), capsuleConfig.m_scale.GetY());

                    if (height <= 0.0f || radius <= 0.0f)
                    {
                        AZ_Error("Jolt Utils", false, "Negative or zero values are invalid for capsule dimensions (height: %f, radius: %f)",
                            capsuleConfig.m_height, capsuleConfig.m_radius)
                        return false;
                    }

                    float halfHeight = 0.5f * height - radius;
                    if (halfHeight <= 0.0f)
                    {
                        AZ_Warning("Jolt", halfHeight < 0.0f, "Height must exceed twice the radius in capsule configuration (height: %f, radius: %f)",
                            capsuleConfig.m_height, capsuleConfig.m_radius)
                        halfHeight = std::numeric_limits<float>::epsilon();
                    }
                    
                    outShape = new JPH::CapsuleShape(halfHeight, radius);
                    break;
                }
            case Physics::ShapeType::PhysicsAsset:
                {
                    AZ_Assert(false,
                        "CreateJoltShapeFromConfig: Cannot pass PhysicsAsset configuration since it is a collection of shapes. "
                        "Please iterate over m_colliderShapes in the asset and call this function for each of them.")
                    return false;
                }
            case Physics::ShapeType::Heightfield:
                {
                    const auto& constHeightfieldConfig =
                        dynamic_cast<const Physics::HeightfieldShapeConfiguration&>(shapeConfiguration);

                    // We are deliberately removing the const off of the ShapeConfiguration here because we're going to change the cached
                    // native heightfield pointer that gets stored in the configuration.
                    auto& heightfieldConfig =
                        const_cast<Physics::HeightfieldShapeConfiguration&>(constHeightfieldConfig);

                    CreateJoltShapeResultFromHeightField(heightfieldConfig, outShape);
                    break;
                }
            default:
                AZ_Warning("Jolt Rigid Body", false, "Shape not supported in Jolt. Shape Type: %d", shapeType)
                return false;
            }

            return true;
        }

        void CreateJoltShapeResultFromHeightField(Physics::HeightfieldShapeConfiguration& heightfieldConfig, JPH::Shape* outShape)
        {
            // Most of this is borrowed from PhysX gem
            const AZ::Vector2& gridSpacing = heightfieldConfig.GetGridResolution();

            const size_t numCols = heightfieldConfig.GetNumColumnVertices();
            const size_t numRows = heightfieldConfig.GetNumRowVertices();
            
            const float rowScale = gridSpacing.GetX();
            const float colScale = gridSpacing.GetY();

            AZ_UNUSED(rowScale)
            AZ_UNUSED(colScale)

            const float minHeightBounds = heightfieldConfig.GetMinHeightBounds();
            const float maxHeightBounds = heightfieldConfig.GetMaxHeightBounds();
            const float halfBounds{ (maxHeightBounds - minHeightBounds) / 2.0f };

            // We're making the assumption right now that the min/max bounds are centered around 0.
            AZ_Assert(
                AZ::IsClose(-halfBounds, minHeightBounds) && AZ::IsClose(halfBounds, maxHeightBounds),
                "Min/Max height bounds aren't centered around 0, the height conversions below will be incorrect.")

            AZ_Assert(
                maxHeightBounds >= minHeightBounds,
                "Max height bounds is less than min height bounds, the height conversions below will be incorrect.")

            // Jolt quantizes float height values into uin16 internally, so we only need to worry about converting floats

            if (auto cachedHeightField = static_cast<JPH::HeightFieldShape*>(heightfieldConfig.GetCachedNativeHeightfield()))
            {                
                outShape = cachedHeightField;
                return;
            }

            AZStd::vector<float> joltHeightSamples = ConvertHeightfieldSamples(heightfieldConfig, 0, 0, numCols, numRows);
 
            // TODO: Determine how or if we can set HeightField offset or scale in O3DE
            JPH::HeightFieldShapeSettings settings(
                joltHeightSamples.data(),
                JPH::Vec3::sZero(),
                JPH::Vec3::sOne(),
                static_cast<JPH::uint32>(joltHeightSamples.size()));
            JPH::Shape::ShapeResult result = settings.Create();
            outShape = result.Get().GetPtr(); // Not sure if this pointer is valid after scope end
        }

        AZStd::vector<float> ConvertHeightfieldSamples(const Physics::HeightfieldShapeConfiguration& heightfield,
            const size_t startCol, const size_t startRow, const size_t numColsToUpdate, const size_t numRowsToUpdate)
        {
            const size_t numCols = heightfield.GetNumColumnVertices();
            const size_t numRows = heightfield.GetNumRowVertices();

            AZ_Assert(startRow < numRows, "Invalid starting row (%d vs %d total rows)", startRow, numRows)
            AZ_Assert(startCol < numCols, "Invalid starting column (%d vs %d total columns)", startCol, numCols)
            AZ_Assert((startRow + numRowsToUpdate) <= numRows, "Invalid row selection")
            AZ_Assert((startCol + numColsToUpdate) <= numCols, "Invalid column selection")

            // Vector of O3DE format samples
            const AZStd::vector<Physics::HeightMaterialPoint>& samples = heightfield.GetSamples();
            AZ_Assert(samples.size() == numRows * numCols, "Heightfield configuration has invalid heightfield sample size.")

            if (samples.empty() || (numRowsToUpdate == 0) || (numColsToUpdate == 0))
            {
                return {};
            }

            const float minHeightBounds = heightfield.GetMinHeightBounds();
            const float maxHeightBounds = heightfield.GetMaxHeightBounds();
            const float halfBounds{ (maxHeightBounds - minHeightBounds) / 2.0f };

            // We're making the assumption that the min/max bounds are centered around 0
            AZ_Assert(
                AZ::IsClose(-halfBounds, minHeightBounds) && AZ::IsClose(halfBounds, maxHeightBounds),
                "Min/Max height bounds aren't centered around 0, the height conversions below will be incorrect.")

            AZ_Assert(
                maxHeightBounds >= minHeightBounds,
                "Max height bounds is less than min height bounds, the height conversions below will be incorrect.")

            // Vector of Jolt format samples
            AZStd::vector<float> heightSamples(numRowsToUpdate * numColsToUpdate);

            for (size_t row = 0; row < numRowsToUpdate; row++)
            {
                for (size_t col = 0; col < numColsToUpdate; col++)
                {
                    const size_t sampleIndex = ((row + startRow) * numCols) + (col + startCol);
                    const size_t joltSampleIndex = (row * numColsToUpdate) + col;

                    // We're populating both format vectors
                    const Physics::HeightMaterialPoint& currentSample = samples[sampleIndex];
                    float& currentJoltSample = heightSamples[joltSampleIndex];

                    // samples are floats in Jolt, so this shouldn't need to use azlossy_cast and scaling
                    currentJoltSample = AZ::GetClamp(currentSample.m_height, minHeightBounds, maxHeightBounds);

                    // TODO: Future me should configure this to work with Jolt Materials in a HeightField
                    // auto [materialIndex0, materialIndex1] =
                    //     GetPhysXMaterialIndicesFromHeightfieldSamples(samples, col + startCol, row + startRow, numCols, numRows);
                    // currentJoltSample.materialIndex0 = materialIndex0;
                    // currentJoltSample.materialIndex1 = materialIndex1;

                    // if (currentSample.m_quadMeshType == Physics::QuadMeshType::SubdivideUpperLeftToBottomRight)
                    // {
                    //     // Set the tesselation flag to say that we need to go from UL to BR
                    //
                    // }
                }
            }

            return heightSamples;
        }

        JPH::Shape* CreateJoltShapeFromConfig(const Physics::ColliderConfiguration& colliderConfiguration,
                                              const Physics::ShapeConfiguration& shapeConfiguration,
                                              AzPhysics::CollisionGroup& assignedCollisionGroup)
        {
            JPH::Shape* outShape = nullptr;
            if (!Utils::ComputeJoltShapeFromConfig(shapeConfiguration, outShape))
            {
                return outShape;
            }
            
            // We get the materials from the collider config here and extract them to set on a shape
            // We can't set Jolt materials on base shapes because we need to know the type
            // We know the type when getting the shape result, but we can't set density there until Create() is called and IsValid()
            // One option is to pass in the joltMaterials vector above and return a fully baked shape that has material and density set
            AZStd::vector<AZStd::shared_ptr<Material>> materials = Material::FindOrCreateMaterials(colliderConfiguration.m_materialSlots);
            AZStd::vector<const JoltPhysicsMaterial*> joltMaterials(materials.size(), nullptr);
            for (size_t materialIndex = 0; materialIndex < materials.size(); ++materialIndex)
            {
                joltMaterials[materialIndex] = materials[materialIndex]->GetJoltMaterial();
            }

            switch (auto shapeType = outShape->GetType())
            {
            case JPH::EShapeType::Convex:
                {
                    dynamic_cast<JPH::ConvexShape*>(outShape)->SetMaterial(joltMaterials.front());
                    dynamic_cast<JPH::ConvexShape*>(outShape)->SetDensity(joltMaterials.front()->GetDensity());
                }
            case JPH::EShapeType::Plane:
                {
                    dynamic_cast<JPH::PlaneShape*>(outShape)->SetMaterial(joltMaterials.front());
                }
            case JPH::EShapeType::Mesh:
                {
                    AZ_Warning("Jolt Rigid Body", false, "Shape not yet supported in Jolt. Shape Type: %d", shapeType)
                    // Materials are per subshape on a Mesh
                }
            case JPH::EShapeType::HeightField:
                {
                    AZ_Warning("Jolt Rigid Body", false, "Shape not yet supported in Jolt. Shape Type: %d", shapeType)
                    // dynamic_cast<JPH::HeightFieldShape*>(outShape)->SetMaterials();
                }
            case JPH::EShapeType::SoftBody:
                {
                    AZ_Warning("Jolt Rigid Body", false, "Shape not yet supported in Jolt. Shape Type: %d", shapeType)
                    // Materials are per subshape on a SoftBody
                }
            case JPH::EShapeType::Empty:
                {
                    AZ_Warning("Jolt Rigid Body", false, "Shape not yet supported in Jolt. Shape Type: %d", shapeType)
                }
            default:
                {
                    AZ_Warning("Jolt Rigid Body", false, "Shape not supported in Jolt. Shape Type: %d", shapeType)
                }
            }

            if (outShape == nullptr)
            {
                AZ_Error("Jolt Rigid Body", false, "Failed to create shape.")
                return outShape;
            }
             
            AzPhysics::CollisionGroup collisionGroup;
            Physics::CollisionRequestBus::BroadcastResult(collisionGroup, &Physics::CollisionRequests::GetCollisionGroupById, colliderConfiguration.m_collisionGroupId);

            // TODO: Set the ObjectLayer bitmask here (Group, layer, BroadPhaseLayer)
            
            assignedCollisionGroup = collisionGroup;

            outShape->AddRef();
            return outShape;
        }
    }
}
