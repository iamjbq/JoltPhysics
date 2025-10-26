
#include <Clients/Shape.h>
#include <Utils.h>

#include <JoltPhysics/Material/JoltMaterial.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>

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

    AzPhysics::CollisionLayer Shape::GetCollisionLayer() const
    {
        return m_collisionLayer;
    }

    AzPhysics::CollisionGroup Shape::GetCollisionGroup() const
    {
        return m_collisionGroup;
    }

    void Shape::SetName(const char* name)
    {
        AZ_Warning("Jolt Shape", false, "Jolt Shapes cannot have names set.")
        AZ_UNUSED(name)
    }

    AZ::Crc32 Shape::GetTag() const
    {
        return m_tag;
    }

    AZStd::shared_ptr<Physics::ShapeConfiguration> Shape::GetShapeConfiguration() const
    {
        return m_shapeConfiguration;
    }

    JPH::Shape* Shape::GetJoltShape()
    {
        if (m_joltShape)
        {
            return m_joltShape.get();
        }
        return nullptr;
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
