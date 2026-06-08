#include <Clients/Shape.h>

#include <Jolt/Physics/Body/Body.h>
#include "Jolt/Physics/PhysicsSystem.h"
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include "Jolt/Physics/Collision/CollisionCollectorImpl.h"

#include <Utils.h>

#include <AzFramework/Physics/CollisionBus.h>
#include <AzCore/Component/EntityBus.h>

#include <JoltPhysics/Utils.h>
#include <JoltPhysics/Material/JoltMaterial.h>
#include "JoltPhysics/MathConversions.h"
#include "JoltPhysics/BodyData.h"
#include "System/JoltSystem.h"

namespace JoltPhysics
{
    Shape::Shape(const Physics::ColliderConfiguration& colliderConfiguration,
                 const Physics::ShapeConfiguration& configuration)
        : m_collisionLayer(colliderConfiguration.m_collisionLayer)
    {
        m_shapeConfiguration = configuration.Clone();
        m_isTrigger = colliderConfiguration.m_isTrigger;
        
        if (auto newShape = Utils::CreateJoltShapeFromConfig(colliderConfiguration, configuration))
        {
            m_joltShape = JoltShapeUniquePtr(newShape, AZStd::bind(&Shape::ReleaseJoltShape, this, newShape));
            m_joltShape->SetUserData(reinterpret_cast<AZ::u64>(this));

            Physics::CollisionRequestBus::BroadcastResult(m_collisionGroup,
                                                          &Physics::CollisionRequests::GetCollisionGroupById,
                                                          colliderConfiguration.m_collisionGroupId);

            m_tag = AZ::Crc32(colliderConfiguration.m_tag);
        }
        else
        {
            AZ_Warning("JoltPhysics::Shape", false, "New shape creation failed")
        }
    }

    Shape::Shape(JPH::Shape* nativeShape)
    {
        m_joltShape = JoltShapeUniquePtr(nativeShape, AZStd::bind(&Shape::ReleaseJoltShape, this, nativeShape));
        m_joltShape->AddRef();
        m_joltShape->SetUserData(reinterpret_cast<AZ::u64>(this));
    }

    Shape::~Shape()
    {
        for (auto material : m_materials)
        {
            material->GetJoltMaterial()->Release();
        }
        m_materials.clear();
        m_joltShape.release();
        m_attachedBody = nullptr;
    }

    Shape::Shape(Shape&& shape) noexcept
        : m_joltShape(std::move(shape.m_joltShape))
          , m_materials(AZStd::move(shape.m_materials))
          , m_collisionLayer(AZStd::move(shape.m_collisionLayer))
          , m_collisionGroup(AZStd::move(shape.m_collisionGroup))
    {
        if (m_joltShape)
        {
            m_joltShape->SetUserData(reinterpret_cast<AZ::u64>(this));
        }
    }

    Shape& Shape::operator=(Shape&& shape) noexcept
    {
        m_joltShape = AZStd::move(shape.m_joltShape);
        m_materials = AZStd::move(shape.m_materials);
        m_collisionLayer = AZStd::move(shape.m_collisionLayer);
        m_collisionGroup = AZStd::move(shape.m_collisionGroup);

        if (m_joltShape)
        {
            m_joltShape->SetUserData(reinterpret_cast<AZ::u64>(this));
        }

        return *this;
    }

    void Shape::SetMaterial(const AZStd::shared_ptr<Physics::Material>& material)
    {
        if (auto materialWrapper = AZStd::rtti_pointer_cast<JoltPhysics::Material>(material))
        {
            m_materials.clear();
            m_materials.emplace_back(materialWrapper);

            BindMaterialsWithJoltShape();
        }
        else
        {
            AZ_Warning("Jolt Shape", false, "Trying to assign material of unknown type")
        }
    }

    AZStd::shared_ptr<Physics::Material> Shape::GetMaterial() const
    {
        if (!m_materials.empty())
        {
            return m_materials[0];
        }
        return nullptr;
    }

    Physics::MaterialId Shape::GetMaterialId() const
    {
        if (!m_materials.empty())
        {
            return m_materials[0]->GetId();
        }

        return {};
    }

    void Shape::SetCollisionLayer(const AzPhysics::CollisionLayer& layer)
    {
        m_collisionLayer = layer;

        AZ_Warning("Shape", false, "SetCollisionLayer currently not implemented on native shape")
    }

    AzPhysics::CollisionLayer Shape::GetCollisionLayer() const
    {
        return m_collisionLayer;
    }

    void Shape::SetCollisionGroup(const AzPhysics::CollisionGroup& group)
    {
        m_collisionGroup = group;

        AZ_Warning("JoltPhysics::Shape", false, "SetCollisionGroup currently not implemented on native shape")
    }

    AzPhysics::CollisionGroup Shape::GetCollisionGroup() const
    {
        return m_collisionGroup;
    }

    void Shape::SetName([[maybe_unused]] const char* name)
    {
        AZ_Warning("JoltPhysics::Shape", false, "Jolt Shapes cannot have names set.")
    }

    void Shape::SetLocalPose([[maybe_unused]] const AZ::Vector3& offset,
                             [[maybe_unused]] const AZ::Quaternion& rotation)
    {
        if (m_joltShape)
        {
            // TODO: Figure out how TransformedShapeCollector works
            // m_joltShape->TransformShape();
        }

        AZ_Warning("JoltPhysics::Shape", false, "SetLocalPose not currently implemented")
    }

    AZStd::pair<AZ::Vector3, AZ::Quaternion> Shape::GetLocalPose() const
    {
        // TODO: confirm if this is correct or should be not transformed, but near origin

        if (!m_attachedBody->GetID().IsInvalid() && m_attachedSystem != nullptr)
        {
            const JPH::TransformedShape transformedShape = m_attachedSystem->GetBodyInterface().GetTransformedShape(
                m_attachedBody->GetID());
            const JPH::Mat44 comTransform = transformedShape.GetCenterOfMassTransform();

            return {JoltMathConvert(comTransform.GetTranslation()), JoltMathConvert(comTransform.GetQuaternion())};
        }
        return {AZ::Vector3::CreateZero(), AZ::Quaternion::CreateZero()};
    }

    float Shape::GetRestOffset() const
    {
        AZ_Warning("JoltPhysics::Shape", false, "GetRestOffset not currently implemented")
        return 0.f;
    }

    float Shape::GetContactOffset() const
    {
        AZ_Warning("JoltPhysics::Shape", false, "GetContactOffset not currently implemented")
        return 0.f;
    }

    void Shape::SetRestOffset([[maybe_unused]] float restOffset)
    {
        AZ_Warning("JoltPhysics::Shape", false, "SetRestOffset not currently implemented")
    }

    void Shape::SetContactOffset([[maybe_unused]] float contactOffset)
    {
        AZ_Warning("JoltPhysics::Shape", false, "SetContactOffset not currently implemented")
    }

    void* Shape::GetNativePointer()
    {
        return m_joltShape.get();
    }

    const void* Shape::GetNativePointer() const
    {
        return m_joltShape.get();
    }

    AZ::Crc32 Shape::GetTag() const
    {
        return m_tag;
    }

    void Shape::AttachedToActor(void* actor)
    {
        JPH::Body* joltBody = static_cast<JPH::Body*>(actor);
        if (joltBody != nullptr)
        {
            m_attachedBody = joltBody;
        }
    }

    void Shape::DetachedFromActor()
    {
        m_attachedBody = nullptr;
        m_attachedSystem = nullptr;
    }
    
    AzPhysics::SceneQueryHit Shape::RayCast(const AzPhysics::RayCastRequest& worldSpaceRequest,
                                            const AZ::Transform& worldTransform)
    {
        if (m_joltShape)
        {
            if (const bool shouldCollide = worldSpaceRequest.m_collisionGroup.GetMask() & m_collisionLayer.GetMask();
                !shouldCollide)
            {
                return AzPhysics::SceneQueryHit();
            }
            
            /////////////////////////////////////
            /// Debug Printing
            /////////////////////////////////////
            
            AZ_Printf("Shape::RayCast", "Shape Pos: %f, %f, %f", worldTransform.GetTranslation().GetX(), worldTransform.GetTranslation().GetY(), worldTransform.GetTranslation().GetZ())
            AZ_Printf("Shape::RayCast", "Shape COM: %f, %f, %f", m_joltShape->GetCenterOfMass().GetX(), m_joltShape->GetCenterOfMass().GetY(), m_joltShape->GetCenterOfMass().GetZ())
            
            JPH::RayCastResult result;
        
            // Ray for shape
            JPH::RRayCast inRay(
                JoltMathConvert(worldSpaceRequest.m_start),
                JoltMathConvert(worldSpaceRequest.m_direction.GetNormalized()) * worldSpaceRequest.m_distance
            );
        
            // Need center of mass transform, NOT world transform!
            JPH::Mat44 centerOfMass = JoltMathConvert(worldTransform).PreTranslated(m_joltShape->GetCenterOfMass());
            
            AZ_Printf("Shape::RayCast", "Shape COM Translation: %f, %f, %f", centerOfMass.GetTranslation().GetX(), centerOfMass.GetTranslation().GetY(), centerOfMass.GetTranslation().GetZ())
            
            AZ_Printf("Shape::RayCast", "Ray world origin: %f, %f, %f", inRay.mOrigin.GetX(), inRay.mOrigin.GetY(), inRay.mOrigin.GetZ())
            // AZ_Printf("Shape::RayCast", "Ray world direction: %f, %f, %f", inRay.mDirection.GetX(), inRay.mDirection.GetY(), inRay.mDirection.GetZ())
            
            // Take the ray to center of mass space for the shape
            // inRay = inRay.Transformed(centerOfMass.InversedRotationTranslation());
            
            AZ_Printf("Shape::RayCast", "Ray COM origin: %f, %f, %f", inRay.mOrigin.GetX(), inRay.mOrigin.GetY(), inRay.mOrigin.GetZ())
            // AZ_Printf("Shape::RayCast", "Ray COM direction: %f, %f, %f", inRay.mDirection.GetX(), inRay.mDirection.GetY(), inRay.mDirection.GetZ())
            
            // Shape option
            bool hadHit = m_attachedSystem->GetNarrowPhaseQuery().CastRay(inRay, result);
            
            AZ_Printf("Shape::RayCast", "Hit: %s", hadHit ? "true" : "false")
            AZ_Printf("Shape::RayCast", "Valid BodyId: %s", !result.mBodyID.IsInvalid() ? "true" : "false")
            // if (auto body = m_attachedSystem->GetBodyLockInterface().TryGetBody(result.mBodyID))
            // {
            //     AZ_Printf("Shape::RayCast", "Body found")
            // }
            // else
            // {
            //     AZ_Printf("Shape::RayCast", "Body not found")
            // }
            
            // BodyId is valid with NarrowPhaseQuery->CastRay for cube, but invalid for sphere and capsule
            // Body is not active in editor for cube, which is expected. Segfault for sphere and capsule
            // Hit works as expected for cubes, and within the defined shape. Will return false for mesh bounding boxes larger than the collider
            // Currently only returns true from inside a sphere, or capsule not from out
            
            
            /////////////////////////////////////
            /// Debug Printing
            /////////////////////////////////////
            
            //     if (hadHit)
            //     {
            //         AZ_Info("Shape::RayCast", "Hat hit!")
            //         // TODO: put this in a convenience function
            //         AzPhysics::SceneQueryHit returnHit;
            //
            //         returnHit.m_distance = worldSpaceRequest.m_distance * result.mFraction;
            //         returnHit.m_resultFlags |= AzPhysics::SceneQuery::ResultFlags::Distance;
            //
            //         JPH::Vec3 hitPosition = m_attachedSystem->GetBodyInterface().GetWorldTransform(result.mBodyID).
            //                                                   GetTranslation();
            //         returnHit.m_position = JoltMathConvert(hitPosition);
            //         returnHit.m_resultFlags |= AzPhysics::SceneQuery::ResultFlags::Position;
            //
            //         // JPH::Vec3 hitNormal = m_attachedSystem->GetBodyInterface().GetTransformedShape(result.mBodyID).GetWorldSpaceSurfaceNormal(result.mSubShapeID2, hitPosition);
            //         JPH::Vec3 hitNormal = m_attachedBody->GetWorldSpaceSurfaceNormal(result.mSubShapeID2, hitPosition);
            //         // TODO: not threadsafe
            //         returnHit.m_normal = JoltMathConvert(hitNormal);
            //         returnHit.m_resultFlags |= AzPhysics::SceneQuery::ResultFlags::Normal;
            //
            //         if (result.mBodyID.IsInvalid())
            //         {
            //             AZ_Warning("Shape::RayCast", false, "Jolt BodyID was invalid")
            //         }
            //
            //         const JPH::uint64 usrData = m_attachedSystem->GetBodyInterface().GetUserData(result.mBodyID);
            //         auto* bodyData = reinterpret_cast<BodyData*>(usrData);
            //         if (!bodyData->IsValid())
            //         {
            //             AZ_Warning("Shape::RayCast", false, "BodyData is not valid")
            //         }
            //         returnHit.m_bodyHandle = bodyData->GetBodyHandle();
            //         if (returnHit.m_bodyHandle != AzPhysics::InvalidSimulatedBodyHandle)
            //         {
            //             returnHit.m_resultFlags |= AzPhysics::SceneQuery::ResultFlags::BodyHandle;
            //         }
            //
            //         returnHit.m_entityId = bodyData->GetEntityId();
            //         if (returnHit.m_entityId.IsValid())
            //         {
            //             AZ_Printf("Shape::RayCast", "Entity hit: %s", returnHit.m_entityId.ToString().c_str())
            //             returnHit.m_resultFlags |= AzPhysics::SceneQuery::ResultFlags::EntityId;
            //         }
            //
            //         returnHit.m_shape = reinterpret_cast<JoltPhysics::Shape*>(m_attachedSystem->GetBodyInterface().
            //             GetShape(result.mBodyID)->GetUserData());
            //         if (returnHit.m_shape != nullptr)
            //         {
            //             returnHit.m_resultFlags |= AzPhysics::SceneQuery::ResultFlags::Shape;
            //         }
            //
            //         if (!result.mSubShapeID2.IsEmpty())
            //         {
            //             auto* joltMaterial = dynamic_cast<const JoltPhysicsMaterial*>(m_attachedSystem->GetBodyInterface().
            //                 GetMaterial(result.mBodyID, result.mSubShapeID2));
            //             returnHit.m_physicsMaterialId = static_cast<Physics::Material*>(joltMaterial->m_userData)->GetId();
            //         }
            //         else if (returnHit.m_shape != nullptr)
            //         {
            //             returnHit.m_physicsMaterialId = returnHit.m_shape->GetMaterialId();
            //         }
            //         if (returnHit.m_physicsMaterialId.IsValid())
            //         {
            //             returnHit.m_resultFlags |= AzPhysics::SceneQuery::ResultFlags::Material;
            //         }
            //
            //         return returnHit;
            //     }
            
            //     AZ_Warning("Shape::RayCast", false, "No hit returned")
            return AzPhysics::SceneQueryHit();
        }
        
        // AZ_Warning("Shape::RayCast", false, "Jolt shape was null")
        return AzPhysics::SceneQueryHit();
    }

    AzPhysics::SceneQueryHit Shape::RayCastLocal([[maybe_unused]] const AzPhysics::RayCastRequest& localSpaceRequest)
    {
        AZ_Warning("JoltPhysics::Shape", false, "RayCastLocal not currently implemented")
        return AzPhysics::SceneQueryHit();
    }

    AZ::Aabb Shape::GetAabb([[maybe_unused]] const AZ::Transform& worldTransform) const
    {
        if (!m_attachedBody->GetID().IsInvalid() && m_attachedSystem != nullptr)
        {
            const JPH::TransformedShape transformedShape = m_attachedSystem->GetBodyInterface().GetTransformedShape(
                m_attachedBody->GetID());
            return JoltMathConvert(transformedShape.GetWorldSpaceBounds());
        }
        AZ_Warning("Shape::GetAabb", false, "Jolt Shape is null")
        return AZ::Aabb::CreateNull();
    }

    AZ::Aabb Shape::GetAabbLocal() const
    {
        if (m_joltShape)
        {
            return JoltMathConvert(m_joltShape->GetLocalBounds());
        }
        AZ_Warning("Shape::GetAabbLocal", false, "Jolt Shape is null")
        return AZ::Aabb::CreateNull();
    }

    AZStd::shared_ptr<Physics::ShapeConfiguration> Shape::GetShapeConfiguration() const
    {
        return m_shapeConfiguration;
    }

    void Shape::GetGeometry([[maybe_unused]] AZStd::vector<AZ::Vector3>& vertices,
                            [[maybe_unused]] AZStd::vector<AZ::u32>& indices,
                            [[maybe_unused]] const AZ::Aabb* optionalBounds) const
    {
        AZ_Warning("JoltPhysics::Shape", false, "GetGeometry not currently implemented")
    }

    bool Shape::GetIsTrigger() const
    {
        return m_isTrigger;
    }

    void Shape::SetInternalPhysicsSystem(JPH::PhysicsSystem* inSystem)
    {
        m_attachedSystem = inSystem;
    }

    void Shape::BindMaterialsWithJoltShape()
    {
        if (m_joltShape)
        {
            AZStd::vector<const JoltPhysics::JoltPhysicsMaterial*> joltMaterials;
            joltMaterials.reserve(m_materials.size());

            for (const auto& material : m_materials)
            {
                joltMaterials.emplace_back(material->GetJoltMaterial());
            }

            AZ_Warning("Jolt Shape", m_materials.size() < std::numeric_limits<AZ::u16>::max(),
                       "Trying to assign too many materials, cutting down");
            size_t materialsCount = AZStd::GetMin(m_materials.size(),
                                                  static_cast<size_t>(std::numeric_limits<AZ::u16>::max()));
            AZ_UNUSED(materialsCount)
            {
                // PhysX locks scene here
                // m_joltShape->GetPtr()->setMaterials(const_cast<JoltPhysics::Material**>(joltMaterials.data()), static_cast<JPH::uint16>(materialsCount));
                // TODO: not sure how to set this yet, since only ConvexShapes have materials, but there are also planes and several derived shape types
            }
        }
    }

    void Shape::ExtractMaterialsFromJoltShape()
    {
        if (m_joltShape)
        {
            return;
        }

        // const int BufferSize = 100;
        // TODO: Store Geometry on Shape and loop through to get all materials as below
        // AZ_Warning("Jolt Shape", m_joltShape->GetMaterial() < BufferSize, "Shape has too many materials, consider increasing the buffer");
    }

    JoltPhysics::JoltScene* Shape::GetScene()
    {
        if (!m_attachedBody->GetID().IsInvalid())
        {
            auto* bodyData = reinterpret_cast<BodyData*>(m_attachedBody->GetUserData());
            if (!bodyData->IsValid())
            {
                AZ_Warning("Shape::GetScene", false, "BodyData is not valid")
                return nullptr;
            }
            return azrtti_cast<JoltScene*>(bodyData->GetSimulatedBody()->GetScene());
        }
        return nullptr;
    }

    void Shape::ReleaseJoltShape(JPH::Shape* shape)
    {
        shape->Release();
    }
} // JoltPhysics
