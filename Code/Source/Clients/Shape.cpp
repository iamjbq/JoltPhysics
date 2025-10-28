
#include <Clients/Shape.h>
#include <Utils.h>

#include <JoltPhysics/Material/JoltMaterial.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>

#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"

namespace JoltPhysics
{
    Shape::Shape(const Physics::ColliderConfiguration& colliderConfiguration,
        const Physics::ShapeConfiguration& configuration)
        : m_collisionLayer(colliderConfiguration.m_collisionLayer)
    {
        m_shapeConfiguration = configuration.Clone();
        
        if (JPH::Shape* newShape = Utils::CreateJoltShapeFromConfig(colliderConfiguration, configuration, m_collisionGroup))
        {
            m_joltShape = JoltShapeUniquePtr(newShape, AZStd::bind(&Shape::ReleaseJoltShape, this, newShape));
            m_joltShape->SetUserData(reinterpret_cast<uintptr_t>(this));
            
            m_tag = AZ::Crc32(colliderConfiguration.m_tag);
        }

    }

    Shape::Shape(JPH::Shape* nativeShape)
    {
        m_joltShape = JoltShapeUniquePtr(nativeShape, AZStd::bind(&Shape::ReleaseJoltShape, this, nativeShape));
        m_joltShape->AddRef();
        m_joltShape->SetUserData(reinterpret_cast<uintptr_t>(this));
    }

    Shape::~Shape()
    {
        m_joltShape.reset();
        m_joltShape = nullptr;
        m_attachedBody = nullptr;
    }

    Shape::Shape(Shape&& shape)
        : m_joltShape(std::move(shape.m_joltShape))
        , m_materials(AZStd::move(shape.m_materials))
        , m_collisionLayer(AZStd::move(shape.m_collisionLayer))
        , m_collisionGroup(AZStd::move(shape.m_collisionGroup))
    {
        if (m_joltShape)
        {
            m_joltShape->SetUserData(reinterpret_cast<uintptr_t>(this));
        }
    }

    Shape& Shape::operator=(Shape&& shape)
    {
        m_joltShape = AZStd::move(shape.m_joltShape);
        m_materials = AZStd::move(shape.m_materials);
        m_collisionLayer = AZStd::move(shape.m_collisionLayer);
        m_collisionGroup = AZStd::move(shape.m_collisionGroup);

        if (m_joltShape)
        {
            m_joltShape->SetUserData(reinterpret_cast<uintptr_t>(this));
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
            AZ_Warning("Jolt Shape", false, "Trying to assign material of unknown type");
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

    void Shape::SetName(const char* name)
    {
        AZ_Warning("JoltPhysics::Shape", false, "Jolt Shapes cannot have names set.")
        AZ_UNUSED(name)
    }

    void Shape::SetLocalPose([[maybe_unused]] const AZ::Vector3& offset, [[maybe_unused]] const AZ::Quaternion& rotation)
    {
        AZ_Warning("JoltPhysics::Shape", false, "SetLocalPose not currently implemented")
    }

    AZStd::pair<AZ::Vector3, AZ::Quaternion> Shape::GetLocalPose() const
    {
        AZ_Warning("JoltPhysics::Shape", false, "GetLocalPose not currently implemented")
        return AZStd::pair(AZ::Vector3::CreateZero(), AZ::Quaternion::CreateZero());
    }

    float Shape::GetRestOffset() const
    {
        AZ_Warning("JoltPhysics::Shape", false, "GetRestOffset not currently implemented")
        return 0.f;
    }

    float Shape::GetContactOffset() const
    {
        return m_joltShape->GetInnerRadius(); // not sure if this is correct though
    }

    void Shape::SetRestOffset([[maybe_unused]] float restOffset)
    {
    }

    void Shape::SetContactOffset([[maybe_unused]] float contactOffset)
    {
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
    }

    AzPhysics::SceneQueryHit Shape::RayCast([[maybe_unused]] const AzPhysics::RayCastRequest& worldSpaceRequest,
        [[maybe_unused]] const AZ::Transform& worldTransform)
    {
        AZ_Warning("JoltPhysics::Shape", false, "RayCast not currently implemented")
        return AzPhysics::SceneQueryHit();
    }

    AzPhysics::SceneQueryHit Shape::RayCastLocal([[maybe_unused]] const AzPhysics::RayCastRequest& localSpaceRequest)
    {
        AZ_Warning("JoltPhysics::Shape", false, "RayCastLocal not currently implemented")
        return AzPhysics::SceneQueryHit();
    }

    AZ::Aabb Shape::GetAabb([[maybe_unused]] const AZ::Transform& worldTransform) const
    {
        AZ_Warning("JoltPhysics::Shape", false, "GetAabb not currently implemented")
        return AZ::Aabb::CreateNull();
    }

    AZ::Aabb Shape::GetAabbLocal() const
    {
        AZ_Warning("JoltPhysics::Shape", false, "GetAabbLocal not currently implemented")
        return AZ::Aabb::CreateNull();
    }

    AZStd::shared_ptr<Physics::ShapeConfiguration> Shape::GetShapeConfiguration() const
    {
        return m_shapeConfiguration;
    }

    void Shape::GetGeometry([[maybe_unused]] AZStd::vector<AZ::Vector3>& vertices, [[maybe_unused]] AZStd::vector<AZ::u32>& indices,
        [[maybe_unused]] const AZ::Aabb* optionalBounds) const
    {
        AZ_Warning("JoltPhysics::Shape", false, "GetGeometry not currently implemented")
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

            AZ_Warning("Jolt Shape", m_materials.size() < std::numeric_limits<AZ::u16>::max(), "Trying to assign too many materials, cutting down");
            size_t materialsCount = AZStd::GetMin(m_materials.size(), static_cast<size_t>(std::numeric_limits<AZ::u16>::max()));
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
        if (m_joltShape == nullptr)
        {
            return;
        }

        // const int BufferSize = 100;
        // TODO: Store Geometry on Shape and loop through to get all materials as below
        // AZ_Warning("Jolt Shape", m_joltShape->GetMaterial() < BufferSize, "Shape has too many materials, consider increasing the buffer");
    }

    JoltPhysics::JoltScene* Shape::GetScene()
    {
        if (m_attachedBody != nullptr)
        {
            // Bodies aren't in Scenes internally, just in O3DE
            
            // return m_attachedBody->getScene();
        }
        return nullptr;
    }

    void Shape::ReleaseJoltShape(JPH::Shape* shape)
    {
        shape->Release();
    }
} // JoltPhysics
