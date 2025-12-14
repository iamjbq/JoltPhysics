
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
#include <AzFramework/Physics/CollisionBus.h>

#include <Jolt/Jolt.h>
#include "Jolt/Math/Vec3.h"
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/DecoratedShape.h"
#include "Jolt/Physics/Collision/Shape/HeightFieldShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/PlaneShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/SoftBody/SoftBodyShape.h"

#include <Clients/JoltPhysicsSystemComponent.h>
#include <JoltPhysics/Utils.h>
#include <JoltPhysics/Material/JoltMaterialConfiguration.h>
#include <JoltPhysics/MathConversions.h>
#include <JoltPhysics/EditorColliderComponentRequestBus.h>
#include <System/JoltSystem.h>
#include <Clients/Shape.h>
#include <Utils.h>

namespace JoltPhysics
{
    namespace Utils
    {
        AZStd::optional<Physics::CookedMeshShapeConfiguration> CreateJoltCookedMeshConfiguration(
            const AZStd::vector<AZ::Vector3>& points, const AZ::Vector3& scale)
        {
            Physics::CookedMeshShapeConfiguration shapeConfig;

            AZStd::vector<AZ::u8> cookedData;
            bool cookingResult = false;
            Physics::SystemRequestBus::BroadcastResult(cookingResult, &Physics::SystemRequests::CookConvexMeshToMemory,
                points.data(), aznumeric_cast<AZ::u32>(points.size()), cookedData);
            shapeConfig.SetCookedMeshData(cookedData.data(), cookedData.size(),
                Physics::CookedMeshShapeConfiguration::MeshType::Convex);
            shapeConfig.m_scale = scale;

            if (!cookingResult)
            {
                AZ_Error("Jolt", false, "Jolt cooking of mesh data failed");
                return {};
            }

            return shapeConfig;
        }

        // Returns a point list of the frustum extents based on the supplied frustum parameters.
        AZStd::optional<AZStd::vector<AZ::Vector3>> CreatePointsAtFrustumExtents(float height, float bottomRadius,
            float topRadius, AZ::u8 subdivisions)
        {
            AZStd::vector<AZ::Vector3> points;

            if (height <= 0.0f)
            {
                AZ_Error("Jolt", false, "Frustum height %f must be greater than 0.", height);
                return {};
            }

            if (bottomRadius < 0.0f)
            {
                AZ_Error("Jolt", false, "Frustum bottom radius %f must be greater or equal to 0.", bottomRadius);
                return {};
            }
            else if (topRadius < 0.0f)
            {
                AZ_Error("Jolt", false, "Frustum top radius %f must be greater or equal to 0.", topRadius);
                return {};
            }
            else if (bottomRadius == 0.0f && topRadius == 0.0f)
            {
                AZ_Error("Jolt", false, "Either frustum bottom radius or top radius must be greater than to 0.");
                return {};
            }

            if (subdivisions < MinFrustumSubdivisions || subdivisions > MaxFrustumSubdivisions)
            {
                AZ_Error("Jolt", false, "Frustum subdivision count %u is not in [%u, %u] range", subdivisions, MinFrustumSubdivisions, MaxFrustumSubdivisions);
                return {};
            }

            points.reserve(subdivisions * 2);
            const float halfHeight = height * 0.5f;
            const double step = AZ::Constants::TwoPi / aznumeric_cast<double>(subdivisions);

            for (double rad = 0; rad < AZ::Constants::TwoPi; rad += step)
            {
                float x = aznumeric_cast<float>(std::cos(rad));
                float y = aznumeric_cast<float>(std::sin(rad));

                points.emplace_back(x * topRadius, y * topRadius, +halfHeight);
                points.emplace_back(x * bottomRadius, y * bottomRadius, -halfHeight);
            }

            return points;
        }

        AzPhysics::Scene* GetDefaultScene()
        {
            AzPhysics::SceneHandle sceneHandle;
            Physics::DefaultWorldBus::BroadcastResult(sceneHandle, &Physics::DefaultWorldRequests::GetDefaultSceneHandle);

            if (auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get())
            {
                if (auto* scene = physicsSystem->GetScene(sceneHandle))
                {
                    return scene;
                }
            }

            return nullptr;
        }

        bool ComputeJoltShapeFromConfig(
            const Physics::ShapeConfiguration& shapeConfiguration,
            JPH::Shape::ShapeResult& outResult,
            AZStd::vector<const JoltPhysicsMaterial*>& inMaterials)
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
                    
                    JPH::SphereShapeSettings settings(sphereConfig.m_radius * shapeConfiguration.m_scale.GetMaxElement(), inMaterials.front());
                    settings.SetDensity(inMaterials.front()->GetDensity());
                    outResult = settings.Create();
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
                    
                    JPH::BoxShapeSettings settings(
                        JoltMathConvert(boxConfig.m_dimensions * 0.5f * shapeConfiguration.m_scale),
                        JPH::cDefaultConvexRadius,
                        inMaterials.front());
                    settings.SetDensity(inMaterials.front()->GetDensity());
                    outResult = settings.Create();
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
                    
                    JPH::CapsuleShapeSettings settings(halfHeight, radius, inMaterials.front());
                    settings.SetDensity(inMaterials.front()->GetDensity());
                    outResult = settings.Create();
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
                    const auto& constHeightfieldConfig = dynamic_cast<const Physics::HeightfieldShapeConfiguration&>(shapeConfiguration);

                    // We are deliberately removing the const off of the ShapeConfiguration here because we're going to change the cached
                    // native heightfield pointer that gets stored in the configuration.
                    auto& heightfieldConfig = const_cast<Physics::HeightfieldShapeConfiguration&>(constHeightfieldConfig);

                    CreateJoltShapeResultFromHeightField(heightfieldConfig, outResult, inMaterials);
                    
                    break;
                }
            default:
                AZ_Warning("Jolt Rigid Body", false, "Shape not supported in Jolt. Shape Type: %d", shapeType)
                return false;
            }

            return true;
        }

        void CreateJoltShapeResultFromHeightField(
            Physics::HeightfieldShapeConfiguration& heightfieldConfig,
            JPH::Shape::ShapeResult& outResult,
            [[maybe_unused]] AZStd::vector<const JoltPhysicsMaterial*>& inMaterials)
        {
            const AZ::Vector2& gridSpacing = heightfieldConfig.GetGridResolution();

            const size_t numCols = heightfieldConfig.GetNumColumnVertices();
            const size_t numRows = heightfieldConfig.GetNumRowVertices();

            const float rowScale = gridSpacing.GetX();
            const float colScale = gridSpacing.GetY();

            // Temp
            AZ_UNUSED(rowScale)
            AZ_UNUSED(colScale)
            // Temp

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
                outResult.Clear();
                outResult.Set(cachedHeightField);
                return;
            }

            AZStd::vector<float> joltHeightSamples = ConvertHeightfieldSamples(heightfieldConfig, 0, 0, numCols, numRows);

            // TODO: Determine how or if we can set HeightField offset or scale in O3DE
            // TODO: Add material indices and material list
            JPH::HeightFieldShapeSettings settings(
                joltHeightSamples.data(),
                JPH::Vec3::sZero(),
                JPH::Vec3::sOne(),
                static_cast<JPH::uint32>(joltHeightSamples.size()));
            outResult = settings.Create();
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
            // We get the materials from the collider config here and extract them to set on a shape
            // We can't set Jolt materials on base shapes because we need to know the type
            AZStd::vector<AZStd::shared_ptr<Material>> materials = Material::FindOrCreateMaterials(colliderConfiguration.m_materialSlots);
            AZStd::vector<const JoltPhysicsMaterial*> joltMaterials(materials.size(), nullptr);
            for (size_t materialIndex = 0; materialIndex < materials.size(); ++materialIndex)
            {
                joltMaterials[materialIndex] = materials[materialIndex]->GetJoltMaterial();
            }

            JPH::Shape::ShapeResult outResult;
            if (!Utils::ComputeJoltShapeFromConfig(shapeConfiguration, outResult, joltMaterials))
            {
                return nullptr;
            }
            
            if (outResult.HasError()) // This should never be true if the above condition passes
            {
                AZ_Error("Jolt Rigid Body", false, "Failed to create shape.")
                return nullptr;
            }
            
            JPH::Shape* newShape = outResult.Get();
            newShape->AddRef();
            
            AzPhysics::CollisionGroup collisionGroup;
            Physics::CollisionRequestBus::BroadcastResult(collisionGroup, &Physics::CollisionRequests::GetCollisionGroupById, colliderConfiguration.m_collisionGroupId);
            
            assignedCollisionGroup = collisionGroup;
            
            return newShape;
        }

        AZ::u32 ConstructObjectLayer(const AzPhysics::CollisionLayer& assignedCollisionLayer,
                                     const AzPhysics::CollisionGroup& assignedCollisionGroup,
                                     const JPH::BroadPhaseLayer& broadPhaseLayer)
        {
            if (JoltSystem* system = GetJoltSystem())
            {
                AZ::u32 newBPLayer = 1 << static_cast<const AZ::u8>(broadPhaseLayer);
                AZ::u32 newCollisionLayer = assignedCollisionLayer.GetIndex() << 8;

                AZ::u32 collisionGroupIndex = system->GetCollisionGroupIndex(assignedCollisionGroup);
                AZ::u32 newCollisionGroup = collisionGroupIndex << 16;
                AZ_Printf("ConstructObjectLayer", "BPLayer %d, CLayer %d, CGrpIdx %d, CGroup %d", newBPLayer, newCollisionLayer, collisionGroupIndex, newCollisionGroup)
                return newCollisionGroup | newCollisionLayer | newBPLayer; // returned in order of setting in ObjectLayer
            }
            AZ_Warning("Jolt Utils", false, "Failed to Get Jolt System for ObJectLayer")
            return 0;
        }

        AZ::Transform GetColliderLocalTransform(const AZ::Vector3& colliderRelativePosition,
                                                const AZ::Quaternion& colliderRelativeRotation)
        {
            return AZ::Transform::CreateFromQuaternionAndTranslation(colliderRelativeRotation, colliderRelativePosition);
        }

        AZ::Transform GetColliderLocalTransform(const AZ::EntityComponentIdPair& idPair)
        {
            AZ::Quaternion colliderRotation = AZ::Quaternion::CreateIdentity();
            JoltPhysics::EditorColliderComponentRequestBus::EventResult(colliderRotation, idPair, &JoltPhysics::EditorColliderComponentRequests::GetColliderRotation);

            AZ::Vector3 colliderOffset = AZ::Vector3::CreateZero();
            JoltPhysics::EditorColliderComponentRequestBus::EventResult(colliderOffset, idPair, &JoltPhysics::EditorColliderComponentRequests::GetColliderOffset);

            return AZ::Transform::CreateFromQuaternionAndTranslation(colliderRotation, colliderOffset);
        }

        AZ::Transform GetColliderWorldTransform(const AZ::Transform& worldTransform,
            const AZ::Vector3& colliderRelativePosition,
            const AZ::Quaternion& colliderRelativeRotation)
        {
            return worldTransform * GetColliderLocalTransform(colliderRelativePosition, colliderRelativeRotation);
        }

        void ColliderPointsLocalToWorld(AZStd::vector<AZ::Vector3>& pointsInOut,
            const AZ::Transform& worldTransform,
            const AZ::Vector3& colliderRelativePosition,
            const AZ::Quaternion& colliderRelativeRotation,
            const AZ::Vector3& nonUniformScale)
        {
            for (AZ::Vector3& point : pointsInOut)
            {
                point = worldTransform.TransformPoint(nonUniformScale *
                    GetColliderLocalTransform(colliderRelativePosition, colliderRelativeRotation).TransformPoint(point));
            }
        }

        // AZ::Aabb GetPxGeometryAabb(const physx::PxGeometryHolder& geometryHolder,
        //     const AZ::Transform& worldTransform,
        //     const ::Physics::ColliderConfiguration& colliderConfiguration
        // )
        // {
        //     const float boundsInflationFactor = 1.0f;
        //     AZ::Transform overallTransformNoScale = GetColliderWorldTransform(worldTransform,
        //         colliderConfiguration.m_position, colliderConfiguration.m_rotation);
        //     overallTransformNoScale.ExtractUniformScale();
        //     const physx::PxBounds3 bounds = physx::PxGeometryQuery::getWorldBounds(geometryHolder.any(),
        //         PxMathConvert(overallTransformNoScale),
        //         boundsInflationFactor);
        //     return PxMathConvert(bounds);
        // }

        // AZ::Aabb GetColliderAabb(const AZ::Transform& worldTransform,
        //     bool hasNonUniformScale,
        //     AZ::u8 subdivisionLevel,
        //     const ::Physics::ShapeConfiguration& shapeConfiguration,
        //     const ::Physics::ColliderConfiguration& colliderConfiguration)
        // {
        //     const AZ::Aabb worldPosAabb = AZ::Aabb::CreateFromPoint(worldTransform.GetTranslation());
        //     physx::PxGeometryHolder geometryHolder;
        //     bool isAssetShape = shapeConfiguration.GetShapeType() == Physics::ShapeType::PhysicsAsset;
        //
        //     if (!isAssetShape)
        //     {
        //         if (!hasNonUniformScale)
        //         {
        //             if (CreatePxGeometryFromConfig(shapeConfiguration, geometryHolder))
        //             {
        //                 return GetPxGeometryAabb(geometryHolder, worldTransform, colliderConfiguration);
        //             }
        //         }
        //         else
        //         {
        //             auto convexPrimitive = Utils::CreateConvexFromPrimitive(colliderConfiguration, shapeConfiguration, subdivisionLevel, shapeConfiguration.m_scale);
        //             if (convexPrimitive.has_value())
        //             {
        //                 if (CreatePxGeometryFromConfig(convexPrimitive.value(), geometryHolder))
        //                 {
        //                     Physics::ColliderConfiguration colliderConfigurationNoOffset = colliderConfiguration;
        //                     colliderConfigurationNoOffset.m_rotation = AZ::Quaternion::CreateIdentity();
        //                     colliderConfigurationNoOffset.m_position = AZ::Vector3::CreateZero();
        //                     return GetPxGeometryAabb(geometryHolder, worldTransform, colliderConfigurationNoOffset);
        //                 }
        //             }
        //         }
        //         return worldPosAabb;
        //     }
        //     else
        //     {
        //         const Physics::PhysicsAssetShapeConfiguration& physicsAssetConfig =
        //             static_cast<const Physics::PhysicsAssetShapeConfiguration&>(shapeConfiguration);
        //
        //         if (!physicsAssetConfig.m_asset.IsReady())
        //         {
        //             return worldPosAabb;
        //         }
        //
        //         AzPhysics::ShapeColliderPairList colliderShapes;
        //         GetColliderShapeConfigsFromAsset(physicsAssetConfig,
        //             colliderConfiguration,
        //             hasNonUniformScale,
        //             subdivisionLevel,
        //             colliderShapes);
        //
        //         if (colliderShapes.empty())
        //         {
        //             return worldPosAabb;
        //         }
        //
        //         AZ::Aabb aabb = AZ::Aabb::CreateNull();
        //         for (const auto& colliderShape : colliderShapes)
        //         {
        //             if (colliderShape.second &&
        //                 CreatePxGeometryFromConfig(*colliderShape.second, geometryHolder))
        //             {
        //                 aabb.AddAabb(
        //                     GetPxGeometryAabb(geometryHolder, worldTransform, *colliderShape.first)
        //                 );
        //             }
        //             else
        //             {
        //                 return worldPosAabb;
        //             }
        //         }
        //         return aabb;
        //     }
        // }

        // bool TriggerColliderExists(AZ::EntityId entityId)
        // {
        //     AZ::EBusLogicalResult<bool, AZStd::logical_or<bool>> response(false);
        //     PhysX::ColliderShapeRequestBus::EventResult(response,
        //         entityId,
        //         &PhysX::ColliderShapeRequestBus::Events::IsTrigger);
        //     return response.value;
        // }
        
        void GetColliderShapeConfigsFromAsset(const Physics::PhysicsAssetShapeConfiguration& assetConfiguration,
            [[maybe_unused]] const Physics::ColliderConfiguration& originalColliderConfiguration, [[maybe_unused]] bool hasNonUniformScale,
            [[maybe_unused]] AZ::u8 subdivisionLevel, [[maybe_unused]] AzPhysics::ShapeColliderPairList& resultingColliderShapes)
        {
            if (!assetConfiguration.m_asset.IsReady())
            {
                AZ_Error("Jolt", false, "GetColliderShapesFromAsset: Asset %s is not ready."
                    "Please make sure the calling code connects to the AssetBus and "
                    "creates the collider shapes only when OnAssetReady or OnAssetReload is invoked.",
                    assetConfiguration.m_asset.GetHint().c_str());
                return;
            }
        
            // const Pipeline::MeshAsset* asset = assetConfiguration.m_asset.GetAs<Pipeline::MeshAsset>();
        
            // if (!asset)
            // {
            //     AZ_Error("Jolt", false, "GetColliderShapesFromAsset: Mesh Asset %s is null."
            //         "Please check the file is in the correct format. Try to delete it and get AssetProcessor re-create it. "
            //         "The data is loaded in Pipeline::MeshAssetHandler::LoadAssetData()",
            //         assetConfiguration.m_asset.GetHint().c_str());
            //     return;
            // }
        
            // const Pipeline::MeshAssetData& assetData = asset->m_assetData;
            // const Pipeline::MeshAssetData::ShapeConfigurationList& shapeConfigList = assetData.m_colliderShapes;
        
            // resultingColliderShapes.reserve(resultingColliderShapes.size() + shapeConfigList.size());
            //
            // for (size_t shapeIndex = 0; shapeIndex < shapeConfigList.size(); shapeIndex++)
            // {
            //     const Pipeline::MeshAssetData::ShapeConfigurationPair& shapeConfigPair = shapeConfigList[shapeIndex];
            //
            //     AZStd::shared_ptr<Physics::ColliderConfiguration> thisColliderConfiguration =
            //         AZStd::make_shared<Physics::ColliderConfiguration>(originalColliderConfiguration);
            //
            //     AZ::u16 shapeMaterialIndex = assetData.m_materialIndexPerShape[shapeIndex];
            //
            //     // Triangle meshes have material indices cooked in the data.
            //     if (shapeMaterialIndex != Pipeline::MeshAssetData::TriangleMeshMaterialIndex)
            //     {
            //         // Clear the materials that came in from the component collider configuration
            //         thisColliderConfiguration->m_materialSlots.SetSlots(Physics::MaterialDefaultSlot::Default);
            //
            //         // Set the material that is relevant for this specific shape
            //         thisColliderConfiguration->m_materialSlots.SetMaterialAsset(
            //             0,
            //             originalColliderConfiguration.m_materialSlots.GetMaterialAsset(shapeMaterialIndex));
            //     }
            //
            //     // Here we use the collider configuration data saved in the asset to update the one coming from the component
            //     if (const Pipeline::AssetColliderConfiguration* optionalColliderData = shapeConfigPair.first.get())
            //     {
            //         optionalColliderData->UpdateColliderConfiguration(*thisColliderConfiguration);
            //     }
            //
            //     // Update the scale with the data from the asset configuration
            //     AZStd::shared_ptr<Physics::ShapeConfiguration> thisShapeConfiguration = shapeConfigPair.second;
            //     thisShapeConfiguration->m_scale = assetConfiguration.m_scale * assetConfiguration.m_assetScale;
            //
            //     // If the shape is a primitive and there is non-uniform scale, replace it with a convex approximation
            //     if (hasNonUniformScale && Utils::IsPrimitiveShape(*thisShapeConfiguration))
            //     {
            //         auto scaledPrimitive = Utils::CreateConvexFromPrimitive(*thisColliderConfiguration,
            //             *thisShapeConfiguration, subdivisionLevel, thisShapeConfiguration->m_scale);
            //         if (scaledPrimitive.has_value())
            //         {
            //             thisShapeConfiguration = AZStd::make_shared<Physics::CookedMeshShapeConfiguration>(scaledPrimitive.value());
            //             physx::PxGeometryHolder pxGeometryHolder;
            //             CreatePxGeometryFromConfig(*thisShapeConfiguration, pxGeometryHolder);
            //             thisColliderConfiguration->m_rotation = AZ::Quaternion::CreateIdentity();
            //             thisColliderConfiguration->m_position = AZ::Vector3::CreateZero();
            //             resultingColliderShapes.emplace_back(thisColliderConfiguration, thisShapeConfiguration);
            //         }
            //     }
            //     else
            //     {
            //         resultingColliderShapes.emplace_back(thisColliderConfiguration, thisShapeConfiguration);
            //     }
            // }
        }

        void CreateShapesFromAsset(const Physics::PhysicsAssetShapeConfiguration& assetConfiguration,
            const Physics::ColliderConfiguration& originalColliderConfiguration, bool hasNonUniformScale,
            AZ::u8 subdivisionLevel, AZStd::vector<AZStd::shared_ptr<Physics::Shape>>& resultingShapes)
        {
            AzPhysics::ShapeColliderPairList resultingColliderShapeConfigs;
            GetColliderShapeConfigsFromAsset(assetConfiguration, originalColliderConfiguration,
                hasNonUniformScale, subdivisionLevel, resultingColliderShapeConfigs);
        
            resultingShapes.reserve(resultingShapes.size() + resultingColliderShapeConfigs.size());
        
            for (const AzPhysics::ShapeColliderPair& shapeConfigPair : resultingColliderShapeConfigs)
            {
                // Scale the collider offset
                shapeConfigPair.first->m_position *= shapeConfigPair.second->m_scale;
        
                AZStd::shared_ptr<Physics::Shape> shape;
                Physics::SystemRequestBus::BroadcastResult(shape, &Physics::SystemRequests::CreateShape,
                    *shapeConfigPair.first, *shapeConfigPair.second);
        
                if (shape)
                {
                    resultingShapes.emplace_back(shape);
                }
            }
        }

        float GetTransformScale(AZ::EntityId entityId)
        {
            float transformScale = 1.0f;
            AZ::TransformBus::EventResult(transformScale, entityId, &AZ::TransformBus::Events::GetWorldUniformScale);
            return transformScale;
        }

        AZ::Vector3 GetNonUniformScale(AZ::EntityId entityId)
        {
            AZ::Vector3 nonUniformScale = AZ::Vector3::CreateOne();
            AZ::NonUniformScaleRequestBus::EventResult(nonUniformScale, entityId, &AZ::NonUniformScaleRequests::GetScale);
            return nonUniformScale;
        }

        AZ::Vector3 GetOverallScale(AZ::EntityId entityId)
        {
            return GetTransformScale(entityId) * GetNonUniformScale(entityId);
        }

        // TODO:
        namespace Geometry
        {
            PointList GenerateBoxPoints(const AZ::Vector3& min, const AZ::Vector3& max)
            {
                PointList pointList;

                auto size = max - min;

                const auto minSamples = 2.f;
                const auto maxSamples = 8.f;
                const auto desiredSampleDelta = 2.f;

                // How many sample in each axis
                int numSamples[] =
                {
                    static_cast<int>(AZ::GetClamp(size.GetX() / desiredSampleDelta, minSamples, maxSamples)),
                    static_cast<int>(AZ::GetClamp(size.GetY() / desiredSampleDelta, minSamples, maxSamples)),
                    static_cast<int>(AZ::GetClamp(size.GetZ() / desiredSampleDelta, minSamples, maxSamples))
                };

                float sampleDelta[] =
                {
                    size.GetX() / static_cast<float>(numSamples[0] - 1),
                    size.GetY() / static_cast<float>(numSamples[1] - 1),
                    size.GetZ() / static_cast<float>(numSamples[2] - 1),
                };

                for (auto i = 0; i < numSamples[0]; ++i)
                {
                    for (auto j = 0; j < numSamples[1]; ++j)
                    {
                        for (auto k = 0; k < numSamples[2]; ++k)
                        {
                            pointList.emplace_back(
                                min.GetX() + i * sampleDelta[0],
                                min.GetY() + j * sampleDelta[1],
                                min.GetZ() + k * sampleDelta[2]
                            );
                        }
                    }
                }

                return pointList;
            }

            PointList GenerateSpherePoints(float radius)
            {
                PointList points;

                int nSamples = static_cast<int>(radius * 5);
                nSamples = AZ::GetClamp(nSamples, 5, 512);

                // Draw arrows using Fibonacci sphere
                float offset = 2.f / nSamples;
                float increment = AZ::Constants::Pi * (3.f - sqrt(5.f));
                for (int i = 0; i < nSamples; ++i)
                {
                    float phi = ((i + 1) % nSamples) * increment;
                    float y = ((i * offset) - 1) + (offset / 2.f);
                    float r = aznumeric_cast<float>(sqrt(1 - pow(y, 2)));
                    float x = cos(phi) * r;
                    float z = sin(phi) * r;
                    points.emplace_back(x * radius, y * radius, z * radius);
                }
                return points;
            }

            PointList GenerateCylinderPoints(float height, float radius)
            {
                PointList points;
                AZ::Vector3 base(0.f, 0.f, -height * 0.5f);
                AZ::Vector3 radiusVector(radius, 0.f, 0.f);

                const auto sides = AZ::GetClamp(radius, 3.f, 8.f);
                const auto segments = AZ::GetClamp(height * 0.5f, 2.f, 8.f);
                const auto angleDelta = AZ::Quaternion::CreateRotationZ(AZ::Constants::TwoPi / sides);
                const auto segmentDelta = height / (segments - 1);
                for (auto segment = 0; segment < segments; ++segment)
                {
                    for (auto side = 0; side < sides; ++side)
                    {
                        auto point = base + radiusVector;
                        points.emplace_back(point);
                        radiusVector = angleDelta.TransformVector(radiusVector);
                    }
                    base += AZ::Vector3(0, 0, segmentDelta);
                }
                return points;
            }

            // void GetBoxGeometry(const physx::PxBoxGeometry& geometry, AZStd::vector<AZ::Vector3>& vertices, AZStd::vector<AZ::u32>& indices)
            // {
            //     constexpr size_t numVertices = 8;
            //     vertices.reserve(numVertices);
            //
            //     vertices.push_back(AZ::Vector3(-geometry.halfExtents.x, -geometry.halfExtents.y, -geometry.halfExtents.z));
            //     vertices.push_back(AZ::Vector3(geometry.halfExtents.x, -geometry.halfExtents.y, -geometry.halfExtents.z));
            //     vertices.push_back(AZ::Vector3(geometry.halfExtents.x, geometry.halfExtents.y, -geometry.halfExtents.z));
            //     vertices.push_back(AZ::Vector3(-geometry.halfExtents.x, geometry.halfExtents.y, -geometry.halfExtents.z));
            //
            //     vertices.push_back(AZ::Vector3(-geometry.halfExtents.x, -geometry.halfExtents.y, geometry.halfExtents.z));
            //     vertices.push_back(AZ::Vector3(geometry.halfExtents.x, -geometry.halfExtents.y, geometry.halfExtents.z));
            //     vertices.push_back(AZ::Vector3(geometry.halfExtents.x, geometry.halfExtents.y, geometry.halfExtents.z));
            //     vertices.push_back(AZ::Vector3(-geometry.halfExtents.x, geometry.halfExtents.y, geometry.halfExtents.z));
            //
            //     constexpr size_t numIndices = 36;
            //     static const AZ::u32 boxIndices[numIndices] =
            //     {
            //         2, 1, 0,
            //         0, 3, 2,
            //         3, 0, 7,
            //         0, 4, 7,
            //         0, 1, 5,
            //         0, 5, 4,
            //         1, 2, 5,
            //         6, 5, 2,
            //         7, 2, 3,
            //         7, 6, 2,
            //         7, 4, 5,
            //         7, 5, 6
            //     };
            //     indices.reserve(numIndices);
            //     for (int i = 0; i < numIndices; ++i)
            //     {
            //         indices.push_back(boxIndices[i]);
            //     }
            // }
            //
            // void GetCapsuleGeometry(const physx::PxCapsuleGeometry& geometry, AZStd::vector<AZ::Vector3>& vertices, AZStd::vector<AZ::u32>& indices, const AZ::u32 stacks, const AZ::u32 slices)
            // {
            //     const AZ::Vector3 base(0.0, 0.0, -geometry.halfHeight);
            //     const AZ::Vector3 top(0.0, 0.0, geometry.halfHeight);
            //     const float radius = geometry.radius;
            //
            //     // topStack refers to the top row of vertices starting at 0
            //     // get an even number so our caps reach all the way out to sphere radius
            //     const AZ::u32 topStack = stacks % 2 ? stacks + 1 : stacks;
            //     const AZ::u32 midStack = topStack / 2;
            //
            //     vertices.reserve(slices * topStack + 2);
            //     indices.reserve((slices - 1) * topStack * 6);
            //
            //     const float thetaFactor = 1.f / aznumeric_cast<float>(topStack) * AZ::Constants::Pi;
            //     const float phiFactor = 1.f / aznumeric_cast<float>(slices - 1) * AZ::Constants::TwoPi;
            //
            //     // bottom cap
            //     vertices.push_back(base + AZ::Vector3(0.f, 0.f, -radius));
            //     for (size_t stack = 1; stack <= midStack; ++stack)
            //     {
            //         for (size_t i = 0; i < slices; ++i)
            //         {
            //             float theta(aznumeric_cast<float>(stack) * thetaFactor);
            //             float phi(aznumeric_cast<float>(i) * phiFactor);
            //
            //             float sinTheta, cosTheta;
            //             AZ::SinCos(theta, sinTheta, cosTheta);
            //
            //             float sinPhi, cosPhi;
            //             AZ::SinCos(phi, sinPhi, cosPhi);
            //
            //             vertices.push_back(base + AZ::Vector3(sinTheta * cosPhi * radius, sinTheta * sinPhi * radius, -cosTheta * radius));
            //         }
            //     }
            //
            //     // top cap
            //     for (size_t stack = midStack; stack < topStack; ++stack)
            //     {
            //         for (size_t i = 0; i < slices; ++i)
            //         {
            //             float theta(aznumeric_cast<float>(stack) * thetaFactor);
            //             float phi(aznumeric_cast<float>(i) * phiFactor);
            //
            //             float sinTheta, cosTheta;
            //             AZ::SinCos(theta, sinTheta, cosTheta);
            //
            //             float sinPhi, cosPhi;
            //             AZ::SinCos(phi, sinPhi, cosPhi);
            //
            //             vertices.push_back(top + AZ::Vector3(sinTheta * cosPhi * radius, sinTheta * sinPhi * radius, -cosTheta * radius));
            //         }
            //     }
            //     vertices.push_back(top + AZ::Vector3(0.f, 0.f, radius));
            //
            //     const AZ::u32 lastVertex = aznumeric_cast<AZ::u32>(vertices.size()) - 1;
            //     const AZ::u32 topRow = aznumeric_cast<AZ::u32>(vertices.size()) - slices - 1;
            //
            //     // top and bottom segment indices
            //     for (AZ::u32 i = 0; i < slices - 1; ++i)
            //     {
            //         // bottom (add one to account for single bottom vertex)
            //         indices.push_back(0);
            //         indices.push_back(i + 2);
            //         indices.push_back(i + 1);
            //
            //         //top (topRow accounts for the added bottom vertex)
            //         indices.push_back(topRow + i + 0);
            //         indices.push_back(topRow + i + 1);
            //         indices.push_back(lastVertex);
            //     }
            //
            //     // there are stacks + 1 stacks because we stretched the middle for the cylinder section,
            //     // but we already built the top and bottom stack so there are stacks + 1 - 2 to build
            //     // add 1 to each vertex index because there is a single bottom vertex for the bottom cap
            //     for (AZ::u32 j = 0; j < stacks - 1; ++j)
            //     {
            //         for (AZ::u32 i = 0; i < slices - 1; ++i)
            //         {
            //             indices.push_back(j * slices + i + 2);
            //             indices.push_back((j + 1) * slices + i + 2);
            //             indices.push_back((j + 1) * slices + i + 1);
            //             indices.push_back(j * slices + i + 1);
            //             indices.push_back(j * slices + i + 2);
            //             indices.push_back((j + 1) * slices + i + 1);
            //         }
            //     }
            // }
            //
            // void GetConvexMeshGeometry(const physx::PxConvexMeshGeometry& geometry, AZStd::vector<AZ::Vector3>& vertices, [[maybe_unused]] AZStd::vector<AZ::u32>& indices)
            // {
            //     const physx::PxConvexMesh* convexMesh = geometry.convexMesh;
            //     const physx::PxU8* pxIndices = convexMesh->getIndexBuffer();
            //     const physx::PxVec3* pxVertices = convexMesh->getVertices();
            //     const AZ::u32 numPolys = convexMesh->getNbPolygons();
            //
            //     physx::PxHullPolygon poly;
            //     for (AZ::u32 polygonIndex = 0; polygonIndex < numPolys; ++polygonIndex)
            //     {
            //         if (convexMesh->getPolygonData(polygonIndex, poly))
            //         {
            //             constexpr AZ::u32 index1 = 0;
            //             AZ::u32 index2 = 1;
            //             AZ::u32 index3 = 2;
            //
            //             const AZ::Vector3 a = JoltMathConvert(geometry.scale.transform(pxVertices[pxIndices[poly.mIndexBase + index1]]));
            //             const AZ::u32 triangleCount = poly.mNbVerts - 2;
            //
            //             for (AZ::u32 triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
            //             {
            //                 AZ_Assert(index3 < poly.mNbVerts, "Implementation error: attempted to index outside range of polygon vertices.");
            //
            //                 const AZ::Vector3 b = JoltMathConvert(geometry.scale.transform(pxVertices[pxIndices[poly.mIndexBase + index2]]));
            //                 const AZ::Vector3 c = JoltMathConvert(geometry.scale.transform(pxVertices[pxIndices[poly.mIndexBase + index3]]));
            //
            //                 vertices.push_back(a);
            //                 vertices.push_back(b);
            //                 vertices.push_back(c);
            //
            //                 index2 = index3++;
            //             }
            //         }
            //     }
            // }
            //
            // void GetHeightFieldGeometry(const physx::PxHeightFieldGeometry& geometry, AZStd::vector<AZ::Vector3>& vertices,
            //     [[maybe_unused]] AZStd::vector<AZ::u32>& indices, const AZ::Aabb* optionalBounds)
            // {
            //     int minX = 0;
            //     int minY = 0;
            //
            //     // rows map to y and columns to x see EditorTerrainComponent
            //     int maxX = geometry.heightField->getNbColumns() - 1;
            //     int maxY = geometry.heightField->getNbRows() - 1;
            //
            //     if (optionalBounds)
            //     {
            //         // convert the provided bounds to heightfield sample grid positions
            //         const AZ::Aabb bounds = *optionalBounds;
            //         const float inverseRowScale = 1.f / geometry.rowScale;
            //         const float inverseColumnScale = 1.f / geometry.columnScale;
            //
            //         minX = AZStd::max(minX, static_cast<int>(floor(bounds.GetMin().GetX() * inverseColumnScale)));
            //         minY = AZStd::max(minY, static_cast<int>(floor(bounds.GetMin().GetY() * inverseRowScale)));
            //         maxX = AZStd::min(maxX, static_cast<int>(ceil(bounds.GetMax().GetX() * inverseColumnScale)));
            //         maxY = AZStd::min(maxY, static_cast<int>(ceil(bounds.GetMax().GetY() * inverseRowScale)));
            //
            //         // Make sure min values don't exceed the max
            //         minX = AZStd::min(minX, maxX);
            //         minY = AZStd::min(minY, maxY);
            //     }
            //
            //     // num quads * 2 triangles per quad * 3 vertices per triangle
            //     const size_t numVertices = (maxY - minY) * (maxX - minX) * 2 * 3;
            //     vertices.reserve(numVertices);
            //
            //     for (int y = minY; y < maxY; ++y)
            //     {
            //         for (int x = minX; x < maxX; ++x)
            //         {
            //             const physx::PxHeightFieldSample& pxSample = geometry.heightField->getSample(y, x);
            //
            //             if (pxSample.materialIndex0 == physx::PxHeightFieldMaterial::eHOLE ||
            //                 pxSample.materialIndex1 == physx::PxHeightFieldMaterial::eHOLE)
            //             {
            //                 // skip terrain geometry marked as eHOLE, this feature is often used for tunnels
            //                 continue;
            //             }
            //
            //             float height = aznumeric_cast<float>(pxSample.height) * geometry.heightScale;
            //
            //             const AZ::Vector3 v0(aznumeric_cast<float>(x) * geometry.rowScale, aznumeric_cast<float>(y) * geometry.columnScale, height);
            //
            //             height = aznumeric_cast<float>(geometry.heightField->getSample(y + 1, x).height) * geometry.heightScale;
            //             const AZ::Vector3 v1(aznumeric_cast<float>(x) * geometry.rowScale, aznumeric_cast<float>(y + 1) * geometry.columnScale, height);
            //
            //             height = aznumeric_cast<float>(geometry.heightField->getSample(y, x + 1).height) * geometry.heightScale;
            //             const AZ::Vector3 v2(aznumeric_cast<float>(x + 1) * geometry.rowScale, aznumeric_cast<float>(y) * geometry.columnScale, height);
            //
            //             height = aznumeric_cast<float>(geometry.heightField->getSample(y + 1, x + 1).height) * geometry.heightScale;
            //             const AZ::Vector3 v3(aznumeric_cast<float>(x + 1) * geometry.rowScale, aznumeric_cast<float>(y + 1) * geometry.columnScale, height);
            //
            //             vertices.push_back(v0);
            //             vertices.push_back(v1);
            //             vertices.push_back(v2);
            //
            //             vertices.push_back(v1);
            //             vertices.push_back(v3);
            //             vertices.push_back(v2);
            //         }
            //     }
            // }
            //
            // void GetSphereGeometry(const physx::PxSphereGeometry& geometry, AZStd::vector<AZ::Vector3>& vertices, AZStd::vector<AZ::u32>& indices, const AZ::u32 stacks, const AZ::u32 slices)
            // {
            //     const float radius = geometry.radius;
            //     const size_t vertexCount = slices * (stacks - 2) + 2;
            //     vertices.reserve(vertexCount);
            //
            //     vertices.push_back(AZ::Vector3(0.f, radius, 0.f));
            //     vertices.push_back(AZ::Vector3(0.f, -radius, 0.f));
            //
            //     for (size_t j = 1; j < stacks - 1; ++j)
            //     {
            //         for (size_t i = 0; i < slices; ++i)
            //         {
            //             float theta = (j / (float)(stacks - 1)) * AZ::Constants::Pi;
            //             float phi = (i / (float)(slices - 1)) * AZ::Constants::TwoPi;
            //
            //             float sinTheta, cosTheta;
            //             AZ::SinCos(theta, sinTheta, cosTheta);
            //
            //             float sinPhi, cosPhi;
            //             AZ::SinCos(phi, sinPhi, cosPhi);
            //
            //             vertices.push_back(AZ::Vector3(sinTheta * cosPhi * radius, cosTheta * radius, -sinTheta * sinPhi * radius));
            //         }
            //     }
            //
            //     const size_t indexCount = (slices - 1) * (stacks - 2) * 6;
            //     indices.reserve(indexCount);
            //
            //     for (AZ::u32 i = 0; i < slices - 1; ++i)
            //     {
            //         indices.push_back(0);
            //         indices.push_back(i + 2);
            //         indices.push_back(i + 3);
            //
            //         indices.push_back((stacks - 3) * slices + i + 3);
            //         indices.push_back((stacks - 3) * slices + i + 2);
            //         indices.push_back(1);
            //     }
            //
            //     for (AZ::u32 j = 0; j < stacks - 3; ++j)
            //     {
            //         for (AZ::u32 i = 0; i < slices - 1; ++i)
            //         {
            //             indices.push_back((j + 1) * slices + i + 3);
            //             indices.push_back(j * slices + i + 3);
            //             indices.push_back((j + 1) * slices + i + 2);
            //             indices.push_back(j * slices + i + 3);
            //             indices.push_back(j * slices + i + 2);
            //             indices.push_back((j + 1) * slices + i + 2);
            //         }
            //     }
            // }
            //
            // void GetTriangleMeshGeometry(const physx::PxTriangleMeshGeometry& geometry, AZStd::vector<AZ::Vector3>& vertices, AZStd::vector<AZ::u32>& indices)
            // {
            //     const physx::PxTriangleMesh* triangleMesh = geometry.triangleMesh;
            //     const physx::PxVec3* meshVertices = triangleMesh->getVertices();
            //     const AZ::u32 vertCount = triangleMesh->getNbVertices();
            //     const AZ::u32 triangleCount = triangleMesh->getNbTriangles();
            //
            //     vertices.reserve(vertCount);
            //     indices.reserve(triangleCount * 3);
            //
            //     for (AZ::u32 vertIndex = 0; vertIndex < vertCount; ++vertIndex)
            //     {
            //         vertices.push_back(JoltMathConvert(geometry.scale.transform(meshVertices[vertIndex])));
            //     }
            //
            //     physx::PxTriangleMeshFlags triangleMeshFlags = triangleMesh->getTriangleMeshFlags();
            //     if (triangleMeshFlags.isSet(physx::PxTriangleMeshFlag::Enum::e16_BIT_INDICES))
            //     {
            //         const physx::PxU16* triangles = static_cast<const physx::PxU16*>(triangleMesh->getTriangles());
            //         for (AZ::u32 triangleIndex = 0; triangleIndex < triangleCount * 3; triangleIndex += 3)
            //         {
            //             indices.push_back(triangles[triangleIndex]);
            //             indices.push_back(triangles[triangleIndex + 1]);
            //             indices.push_back(triangles[triangleIndex + 2]);
            //         }
            //     }
            //     else
            //     {
            //         const physx::PxU32* triangles = static_cast<const physx::PxU32*>(triangleMesh->getTriangles());
            //         for (AZ::u32 triangleIndex = 0; triangleIndex < triangleCount * 3; triangleIndex += 3)
            //         {
            //             indices.push_back(triangles[triangleIndex]);
            //             indices.push_back(triangles[triangleIndex + 1]);
            //             indices.push_back(triangles[triangleIndex + 2]);
            //         }
            //     }
            // }
        } // namespace Geometry
    }
}
