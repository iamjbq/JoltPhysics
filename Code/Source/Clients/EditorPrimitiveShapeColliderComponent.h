
#pragma once

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Component/NonUniformScaleBus.h>
#include <AzCore/Math/Quaternion.h>

#include <AzFramework/Physics/Common/PhysicsEvents.h>
#include <AzFramework/Physics/Components/SimulatedBodyComponentBus.h>
#include <AzFramework/Physics/ShapeConfiguration.h>
#include <AzFramework/Visibility/BoundsBus.h>

#include <AzToolsFramework/API/ComponentEntitySelectionBus.h>
#include <AzToolsFramework/ComponentMode/ComponentModeDelegate.h>
#include <AzToolsFramework/Manipulators/BoxManipulatorRequestBus.h>
#include <AzToolsFramework/Manipulators/ShapeManipulatorRequestBus.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>

#include <JoltPhysics/ColliderShapeBus.h>
#include <JoltPhysics/EditorColliderComponentRequestBus.h>

#include <Editor/DebugDraw.h>

namespace AzPhysics
{
    class SceneInterface;
    struct SimulatedBody;
}

namespace JoltPhysics
{
    //! Edit context wrapper for cylinder specific parameters and cached geometry.
    struct EditorProxyCylinderShapeConfig
    {
        AZ_CLASS_ALLOCATOR(EditorProxyCylinderShapeConfig, AZ::SystemAllocator);
        AZ_TYPE_INFO(EditorProxyCylinderShapeConfig, "{EEB0EF2E-F4AA-432B-B808-A30BDF2283DE}");
        static void Reflect(AZ::ReflectContext* context);

        //! Cylinder specific parameters.
        AZ::u8 m_subdivisionCount = Physics::ShapeConstants::DefaultCylinderSubdivisionCount;
        float m_height = Physics::ShapeConstants::DefaultCylinderHeight;
        float m_radius = Physics::ShapeConstants::DefaultCylinderRadius;

        //! Configuration stores the convex geometry for the cylinder and shape scale.
        Physics::CookedMeshShapeConfiguration m_configuration;
    };

    //! Proxy container for only displaying a specific shape configuration depending on the shapeType selected.
    struct EditorProxyShapeConfig
    {
        AZ_CLASS_ALLOCATOR(EditorProxyShapeConfig, AZ::SystemAllocator);
        AZ_TYPE_INFO(EditorProxyShapeConfig, "{EF83926A-E106-456F-9243-7F4BE890CA46}");
        static void Reflect(AZ::ReflectContext* context);

        EditorProxyShapeConfig() = default;
        EditorProxyShapeConfig(const Physics::ShapeConfiguration& shapeConfiguration);

        Physics::ShapeType m_shapeType = Physics::ShapeType::Box;
        Physics::SphereShapeConfiguration m_sphere;
        Physics::BoxShapeConfiguration m_box;
        Physics::CapsuleShapeConfiguration m_capsule;
        EditorProxyCylinderShapeConfig m_cylinder;
        bool m_hasNonUniformScale = false; //!< Whether there is a non-uniform scale component on this entity.
        AZ::u8 m_subdivisionLevel = 4; //!< The level of subdivision if a primitive shape is replaced with a convex mesh due to scaling.
        Physics::CookedMeshShapeConfiguration m_cookedMesh;

        bool IsSphereConfig() const;
        bool IsBoxConfig() const;
        bool IsCapsuleConfig() const;
        bool IsCylinderConfig() const;
        Physics::ShapeConfiguration& GetCurrent();
        const Physics::ShapeConfiguration& GetCurrent() const;

        AZStd::shared_ptr<Physics::ShapeConfiguration> CloneCurrent() const;

        bool IsNonUniformlyScaledPrimitive() const;

    private:
        bool ShowingSubdivisionLevel() const;
        AZ::u32 OnShapeTypeChanged();
        AZ::u32 OnConfigurationChanged();

        Physics::ShapeType m_lastShapeType = Physics::ShapeType::Box;
    };
    
    class EditorPrimitiveShapeColliderComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , public AzToolsFramework::EditorComponentSelectionRequestsBus::Handler
        , protected DebugDraw::DisplayCallback
        , protected AzToolsFramework::EntitySelectionEvents::Bus::Handler
        , private AzToolsFramework::BoxManipulatorRequestBus::Handler
        , private AzToolsFramework::ShapeManipulatorRequestBus::Handler
        , private AZ::TransformNotificationBus::Handler
        , private JoltPhysics::ColliderShapeRequestBus::Handler
        , private JoltPhysics::EditorColliderComponentRequestBus::Handler
        , private JoltPhysics::EditorPrimitiveColliderComponentRequestBus::Handler
        , private AzPhysics::SimulatedBodyComponentRequestsBus::Handler
        , public AzFramework::BoundsRequestBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(EditorPrimitiveShapeColliderComponent, "{DCC0BBA2-6A41-4A36-B503-1E807E12E7E0}",
            AzToolsFramework::Components::EditorComponentBase);

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        EditorPrimitiveShapeColliderComponent() = default;
        EditorPrimitiveShapeColliderComponent(
            const Physics::ColliderConfiguration& colliderConfiguration,
            const Physics::ShapeConfiguration& shapeConfiguration);

        const EditorProxyShapeConfig& GetShapeConfiguration() const;
        const Physics::ColliderConfiguration& GetColliderConfiguration() const;
        Physics::ColliderConfiguration GetColliderConfigurationScaled() const;
        Physics::ColliderConfiguration GetColliderConfigurationNoOffset() const;

        bool IsDebugDrawDisplayFlagEnabled() const;

        // BoundsRequestBus overrides ...
        AZ::Aabb GetWorldBounds() const override;
        AZ::Aabb GetLocalBounds() const override;

        // EditorComponentSelectionRequestsBus overrides ...
        AZ::Aabb GetEditorSelectionBoundsViewport(const AzFramework::ViewportInfo& viewportInfo) override;
        bool EditorSelectionIntersectRayViewport(
            const AzFramework::ViewportInfo& viewportInfo, const AZ::Vector3& src, const AZ::Vector3& dir, float& distance) override;
        bool SupportsEditorRayIntersect() override;

        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        AZ_DISABLE_COPY_MOVE(EditorPrimitiveShapeColliderComponent);

        // AZ::Component overrides ...
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        void UpdateShapeConfiguration();

        // AzToolsFramework::EntitySelectionEvents overrides ...
        void OnSelected() override;
        void OnDeselected() override;

        // DisplayCallback overrides ...
        void Display(const AzFramework::ViewportInfo& viewportInfo,
            AzFramework::DebugDisplayRequests& debugDisplay) const override;

        void DisplayCylinderCollider(AzFramework::DebugDisplayRequests& debugDisplay) const;
        void DisplayUnscaledPrimitiveCollider(AzFramework::DebugDisplayRequests& debugDisplay) const;
        void DisplayScaledPrimitiveCollider(AzFramework::DebugDisplayRequests& debugDisplay) const;

        // TransformBus overrides ...
        void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;

        // non-uniform scale handling
        void OnNonUniformScaleChanged(const AZ::Vector3& nonUniformScale);

        // AzToolsFramework::BoxManipulatorRequestBus overrides ...
        AZ::Vector3 GetDimensions() const override;
        void SetDimensions(const AZ::Vector3& dimensions) override;
        AZ::Transform GetCurrentLocalTransform() const override;

        // AzToolsFramework::ShapeManipulatorRequestBus overrides ...
        AZ::Vector3 GetTranslationOffset() const override;
        void SetTranslationOffset(const AZ::Vector3& translationOffset) override;
        AZ::Transform GetManipulatorSpace() const override;
        AZ::Quaternion GetRotationOffset() const override;

        // JoltPhysics::ColliderShapeBus overrides ...
        AZ::Aabb GetColliderShapeAabb() override;
        bool IsTrigger() override;

        // JoltPhysics::EditorColliderComponentRequestBus overrides ...
        void SetColliderOffset(const AZ::Vector3& offset) override;
        AZ::Vector3 GetColliderOffset() const override;
        void SetColliderRotation(const AZ::Quaternion& rotation) override;
        AZ::Quaternion GetColliderRotation() const override;
        AZ::Transform GetColliderWorldTransform() const override;
        Physics::ShapeType GetShapeType() const override;

        // JoltPhysics::EditorPrimitiveColliderComponentRequestBus overrides ...
        void SetShapeType(Physics::ShapeType shapeType) override;
        void SetBoxDimensions(const AZ::Vector3& dimensions) override;
        AZ::Vector3 GetBoxDimensions() const override;
        void SetSphereRadius(float radius) override;
        float GetSphereRadius() const override;
        void SetCapsuleRadius(float radius) override;
        float GetCapsuleRadius() const override;
        void SetCapsuleHeight(float height) override;
        float GetCapsuleHeight() const override;
        void SetCylinderRadius(float radius) override;
        float GetCylinderRadius() const override;
        void SetCylinderHeight(float height) override;
        float GetCylinderHeight() const override;
        void SetCylinderSubdivisionCount(AZ::u8 subdivisionCount) override;
        AZ::u8 GetCylinderSubdivisionCount() const override;

        AZ::Transform GetColliderLocalTransform() const;

        AZ::u32 OnConfigurationChanged();
        void UpdateShapeConfigurationScale();

        // AzPhysics::SimulatedBodyComponentRequestsBus::Handler overrides ...
        void EnablePhysics() override;
        void DisablePhysics() override;
        bool IsPhysicsEnabled() const override;
        AZ::Aabb GetAabb() const override;
        AzPhysics::SimulatedBody* GetSimulatedBody() override;
        AzPhysics::SimulatedBodyHandle GetSimulatedBodyHandle() const override;
        AzPhysics::SceneQueryHit RayCast(const AzPhysics::RayCastRequest& request) override;

        // Cylinder collider
        void UpdateCylinderCookedMesh();

        void UpdateCollider();
        void CreateStaticEditorCollider();

        void BuildDebugDrawMesh() const;

        EditorProxyShapeConfig m_proxyShapeConfiguration;
        Physics::ColliderConfiguration m_configuration;

        using ComponentModeDelegate = AzToolsFramework::ComponentModeFramework::ComponentModeDelegate;
        ComponentModeDelegate m_componentModeDelegate; //!< Responsible for detecting ComponentMode activation
                                                       //!< and creating a concrete ComponentMode.

        AzPhysics::SceneInterface* m_sceneInterface = nullptr;
        AzPhysics::SceneHandle m_editorSceneHandle = AzPhysics::InvalidSceneHandle;
        AzPhysics::SimulatedBodyHandle m_editorBodyHandle = AzPhysics::InvalidSimulatedBodyHandle;

        DebugDraw::Collider m_colliderDebugDraw;

        AzPhysics::SystemEvents::OnConfigurationChangedEvent::Handler m_joltConfigChangedHandler;
        AZ::Transform m_cachedWorldTransform;

        AZ::NonUniformScaleChangedEvent::Handler m_nonUniformScaleChangedHandler; //!< Responds to changes in non-uniform scale.
        bool m_hasNonUniformScale = false; //!< Whether there is a non-uniform scale component on this entity.
        AZ::Vector3 m_cachedNonUniformScale = AZ::Vector3::CreateOne(); //!< Caches the current non-uniform scale.
        mutable AZStd::optional<Physics::CookedMeshShapeConfiguration> m_scaledPrimitive; //!< Approximation for non-uniformly scaled primitive.
        AZ::Aabb m_cachedAabb = AZ::Aabb::CreateNull(); //!< Cache the Aabb to avoid recalculating it.
        bool m_cachedAabbDirty = true; //!< Track whether the cached Aabb needs to be recomputed.
    };
} // JoltPhysics
