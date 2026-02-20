
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Configuration/StaticRigidBodyConfiguration.h>

#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>

#include <Editor/ColliderComponentMode.h>
#include <System/JoltSystem.h>

#include <Clients/BoxColliderComponent.h>
#include <Clients/CapsuleColliderComponent.h>
#include <Clients/SphereColliderComponent.h>
#include <Clients/EditorStaticRigidBodyComponent.h>
#include <Utils.h>
#include <Clients/EditorSimpleShapeColliderComponent.h>

namespace JoltPhysics
{
    void EditorProxyCylinderShapeConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorProxyCylinderShapeConfig>()
                ->Version(1)
                ->Field("Configuration", &EditorProxyCylinderShapeConfig::m_configuration)
                ->Field("Subdivision", &EditorProxyCylinderShapeConfig::m_subdivisionCount)
                ->Field("Height", &EditorProxyCylinderShapeConfig::m_height)
                ->Field("Radius", &EditorProxyCylinderShapeConfig::m_radius)
            ;

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<EditorProxyCylinderShapeConfig>("EditorProxyCylinderShapeConfig", "Proxy structure to wrap cylinder data")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorProxyCylinderShapeConfig::m_configuration,
                        "Configuration", "Jolt cylinder collider configuration.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorProxyCylinderShapeConfig::m_subdivisionCount,
                        "Subdivision", "Cylinder subdivision count.")
                        ->Attribute(AZ::Edit::Attributes::Min, Utils::MinFrustumSubdivisions)
                        ->Attribute(AZ::Edit::Attributes::Max, Utils::MaxFrustumSubdivisions)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorProxyCylinderShapeConfig::m_height, "Height", "Cylinder height.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorProxyCylinderShapeConfig::m_radius, "Radius", "Cylinder radius.")
                    ;
            }
        }
    }

    void EditorProxyShapeConfig::Reflect(AZ::ReflectContext* context)
    {
        EditorProxyCylinderShapeConfig::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorProxyShapeConfig>()
                ->Version(1)
                ->Field("ShapeType", &EditorProxyShapeConfig::m_shapeType)
                ->Field("Sphere", &EditorProxyShapeConfig::m_sphere)
                ->Field("Box", &EditorProxyShapeConfig::m_box)
                ->Field("Capsule", &EditorProxyShapeConfig::m_capsule)
                ->Field("Cylinder", &EditorProxyShapeConfig::m_cylinder)
                ->Field("HasNonUniformScale", &EditorProxyShapeConfig::m_hasNonUniformScale)
                ->Field("SubdivisionLevel", &EditorProxyShapeConfig::m_subdivisionLevel)
                ;

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<EditorProxyShapeConfig>(
                    "EditorProxyShapeConfig", "Jolt Base shape collider")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &EditorProxyShapeConfig::m_shapeType, "Shape", "The shape of the collider.")
                        ->EnumAttribute(Physics::ShapeType::Sphere, "Sphere")
                        ->EnumAttribute(Physics::ShapeType::Box, "Box")
                        ->EnumAttribute(Physics::ShapeType::Capsule, "Capsule")
                        ->EnumAttribute(Physics::ShapeType::Cylinder, "Cylinder")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorProxyShapeConfig::OnShapeTypeChanged)
                        // note: we do not want the user to be able to change shape types while in ComponentMode (there will
                        // potentially be different ComponentModes for different shape types)
                        ->Attribute(AZ::Edit::Attributes::ReadOnly, &AzToolsFramework::ComponentModeFramework::InComponentMode)

                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorProxyShapeConfig::m_sphere, "Sphere", "Configuration of sphere shape.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &EditorProxyShapeConfig::IsSphereConfig)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorProxyShapeConfig::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorProxyShapeConfig::m_box, "Box", "Configuration of box shape.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &EditorProxyShapeConfig::IsBoxConfig)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorProxyShapeConfig::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorProxyShapeConfig::m_capsule, "Capsule", "Configuration of capsule shape.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &EditorProxyShapeConfig::IsCapsuleConfig)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorProxyShapeConfig::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorProxyShapeConfig::m_cylinder, "Cylinder", "Configuration of cylinder shape.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &EditorProxyShapeConfig::IsCylinderConfig)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorProxyShapeConfig::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorProxyShapeConfig::m_subdivisionLevel, "Subdivision level",
                        "The level of subdivision if a primitive shape is replaced with a convex mesh due to scaling.")
                        ->Attribute(AZ::Edit::Attributes::Min, Utils::MinCapsuleSubdivisionLevel)
                        ->Attribute(AZ::Edit::Attributes::Max, Utils::MaxCapsuleSubdivisionLevel)
                        ->Attribute(AZ::Edit::Attributes::Visibility, &EditorProxyShapeConfig::ShowingSubdivisionLevel)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorProxyShapeConfig::OnConfigurationChanged)
                    ;
            }
        }
    }

    AZ::u32 EditorProxyShapeConfig::OnShapeTypeChanged()
    {
        m_lastShapeType = m_shapeType;
        return AZ::Edit::PropertyRefreshLevels::EntireTree;
    }

    AZ::u32 EditorProxyShapeConfig::OnConfigurationChanged()
    {
        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    void EditorSimpleShapeColliderComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("PhysicsWorldBodyService"));
        provided.push_back(AZ_CRC_CE("PhysicsColliderService"));
        provided.push_back(AZ_CRC_CE("PhysicsTriggerService"));
    }

    void EditorSimpleShapeColliderComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("TransformService"));
        required.push_back(AZ_CRC_CE("PhysicsRigidBodyService"));
    }

    void EditorSimpleShapeColliderComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("NonUniformScaleService"));
    }

    void EditorSimpleShapeColliderComponent::Reflect(AZ::ReflectContext* context)
    {
        EditorProxyShapeConfig::Reflect(context);
        DebugDraw::Collider::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorSimpleShapeColliderComponent, EditorComponentBase>()
                ->Version(1)
                ->Field("ColliderConfiguration", &EditorSimpleShapeColliderComponent::m_configuration)
                ->Field("ShapeConfiguration", &EditorSimpleShapeColliderComponent::m_proxyShapeConfiguration)
                ->Field("DebugDrawSettings", &EditorSimpleShapeColliderComponent::m_colliderDebugDraw)
                ->Field("ComponentMode", &EditorSimpleShapeColliderComponent::m_componentModeDelegate)
                ->Field("HasNonUniformScale", &EditorSimpleShapeColliderComponent::m_hasNonUniformScale)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<EditorSimpleShapeColliderComponent>(
                    "Jolt Primitive Shape Collider", "Creates geometry in the Jolt simulation using primitive shape.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Jolt")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/PhysXCollider.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Icons/Components/Viewport/PhysXCollider.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "https://www.o3de.org/docs/user-guide/components/reference/physx/collider/")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorSimpleShapeColliderComponent::m_configuration, "Collider Configuration", "Configuration of the collider.")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorSimpleShapeColliderComponent::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorSimpleShapeColliderComponent::m_proxyShapeConfiguration, "Shape Configuration", "Configuration of the shape.")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorSimpleShapeColliderComponent::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorSimpleShapeColliderComponent::m_componentModeDelegate, "Component Mode", "Collider Component Mode.")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &EditorSimpleShapeColliderComponent::m_colliderDebugDraw,
                        "Debug draw settings", "Debug draw settings.")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ;
            }
        }
    }

    EditorSimpleShapeColliderComponent::EditorSimpleShapeColliderComponent(
        const Physics::ColliderConfiguration& colliderConfiguration,
        const Physics::ShapeConfiguration& shapeConfiguration)
        : m_proxyShapeConfiguration(shapeConfiguration)
        , m_configuration(colliderConfiguration)
    {

    }

    const EditorProxyShapeConfig& EditorSimpleShapeColliderComponent::GetShapeConfiguration() const
    {
        return m_proxyShapeConfiguration;
    }

    const Physics::ColliderConfiguration& EditorSimpleShapeColliderComponent::GetColliderConfiguration() const
    {
        return m_configuration;
    }

    Physics::ColliderConfiguration EditorSimpleShapeColliderComponent::GetColliderConfigurationScaled() const
    {
        // Scale the collider offset
        Physics::ColliderConfiguration colliderConfiguration = m_configuration;
        colliderConfiguration.m_position *= Utils::GetTransformScale(GetEntityId()) * m_cachedNonUniformScale;
        return colliderConfiguration;
    }

    Physics::ColliderConfiguration EditorSimpleShapeColliderComponent::GetColliderConfigurationNoOffset() const
    {
        Physics::ColliderConfiguration colliderConfiguration = m_configuration;
        colliderConfiguration.m_position = AZ::Vector3::CreateZero();
        colliderConfiguration.m_rotation = AZ::Quaternion::CreateIdentity();
        return colliderConfiguration;
    }

    bool EditorSimpleShapeColliderComponent::IsDebugDrawDisplayFlagEnabled() const
    {
        return m_colliderDebugDraw.IsDisplayFlagEnabled();
    }

    EditorProxyShapeConfig::EditorProxyShapeConfig(const Physics::ShapeConfiguration& shapeConfiguration)
    {
        m_shapeType = shapeConfiguration.GetShapeType();
        switch (m_shapeType)
        {
        case Physics::ShapeType::Sphere:
            m_sphere = static_cast<const Physics::SphereShapeConfiguration&>(shapeConfiguration);
            break;
        case Physics::ShapeType::Box:
            m_box = static_cast<const Physics::BoxShapeConfiguration&>(shapeConfiguration);
            break;
        case Physics::ShapeType::Capsule:
            m_capsule = static_cast<const Physics::CapsuleShapeConfiguration&>(shapeConfiguration);
            break;
        case Physics::ShapeType::CookedMesh:
            m_cookedMesh = static_cast<const Physics::CookedMeshShapeConfiguration&>(shapeConfiguration);
            break;
        default:
            AZ_Warning("EditorProxyShapeConfig", false, "Invalid shape type!");
        }
    }

    bool EditorProxyShapeConfig::IsSphereConfig() const
    {
        return m_shapeType == Physics::ShapeType::Sphere;
    }

    bool EditorProxyShapeConfig::IsBoxConfig() const
    {
        return m_shapeType == Physics::ShapeType::Box;
    }

    bool EditorProxyShapeConfig::IsCapsuleConfig() const
    {
        return m_shapeType == Physics::ShapeType::Capsule;
    }

    bool EditorProxyShapeConfig::IsCylinderConfig() const
    {
        return m_shapeType == Physics::ShapeType::Cylinder;
    }

    Physics::ShapeConfiguration& EditorProxyShapeConfig::GetCurrent()
    {
        return const_cast<Physics::ShapeConfiguration&>(static_cast<const EditorProxyShapeConfig&>(*this).GetCurrent());
    }

    const Physics::ShapeConfiguration& EditorProxyShapeConfig::GetCurrent() const
    {
        switch (m_shapeType)
        {
        case Physics::ShapeType::Sphere:
            return m_sphere;
        case Physics::ShapeType::Box:
            return m_box;
        case Physics::ShapeType::Capsule:
            return m_capsule;
        case Physics::ShapeType::Cylinder:
            return m_cylinder.m_configuration;
        case Physics::ShapeType::CookedMesh:
            return m_cookedMesh;
        default:
            AZ_Warning("EditorProxyShapeConfig", false, "Unsupported shape type");
            return m_box;
        }
    }

    AZStd::shared_ptr<Physics::ShapeConfiguration> EditorProxyShapeConfig::CloneCurrent() const
    {
        switch (m_shapeType)
        {
        case Physics::ShapeType::Sphere:
            return AZStd::make_shared<Physics::SphereShapeConfiguration>(m_sphere);
        case Physics::ShapeType::Capsule:
            return AZStd::make_shared<Physics::CapsuleShapeConfiguration>(m_capsule);
        case Physics::ShapeType::Cylinder:
            return AZStd::make_shared<Physics::CookedMeshShapeConfiguration>(m_cylinder.m_configuration);
        case Physics::ShapeType::CookedMesh:
            return AZStd::make_shared<Physics::CookedMeshShapeConfiguration>(m_cookedMesh);
        default:
            AZ_Warning("EditorProxyShapeConfig", false, "Unsupported shape type, defaulting to Box.");
            [[fallthrough]];
        case Physics::ShapeType::Box:
            return AZStd::make_shared<Physics::BoxShapeConfiguration>(m_box);
        }
    }

    bool EditorProxyShapeConfig::IsNonUniformlyScaledPrimitive() const
    {
        return m_hasNonUniformScale && (Utils::IsPrimitiveShape(GetCurrent()) || IsCylinderConfig());
    }

    bool EditorProxyShapeConfig::ShowingSubdivisionLevel() const
    {
        return m_hasNonUniformScale && (IsCapsuleConfig() || IsSphereConfig());
    }

    void EditorSimpleShapeColliderComponent::Init()
    {
        // O3DE_DEPRECATION_NOTICE(GHI-14718)
        // If initial value is PhysicsAsset, default to Box and update the UI.
        // To be removed when m_shapeType intial value is changed to Box with GHI-14718.
        if (m_proxyShapeConfiguration.m_shapeType == Physics::ShapeType::PhysicsAsset)
        {
            m_proxyShapeConfiguration.m_shapeType = Physics::ShapeType::Box;
            // Primitive colliders can only have one material slot.
            if (m_configuration.m_materialSlots.GetSlotsCount() > 1)
            {
                m_configuration.m_materialSlots.SetSlots(Physics::MaterialDefaultSlot::Default);
            }
            AzToolsFramework::PropertyEditorGUIMessages::Bus::Broadcast(
                &AzToolsFramework::PropertyEditorGUIMessages::RequestRefresh,
                AzToolsFramework::PropertyModificationRefreshLevel::Refresh_AttributesAndValues);
        }
    }

    void EditorSimpleShapeColliderComponent::Activate()
    {
        m_sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
        if (m_sceneInterface)
        {
            m_editorSceneHandle = m_sceneInterface->GetSceneHandle(AzPhysics::EditorPhysicsSceneName);
        }

        m_joltConfigChangedHandler = AzPhysics::SystemEvents::OnConfigurationChangedEvent::Handler(
            []([[maybe_unused]] const AzPhysics::SystemConfiguration* config)
            {
                AzToolsFramework::PropertyEditorGUIMessages::Bus::Broadcast(&AzToolsFramework::PropertyEditorGUIMessages::RequestRefresh,
                    AzToolsFramework::PropertyModificationRefreshLevel::Refresh_AttributesAndValues);
            });

        const auto entityId = GetEntityId();
        const auto componentId = GetId();

        AzToolsFramework::Components::EditorComponentBase::Activate();
        AzToolsFramework::EntitySelectionEvents::Bus::Handler::BusConnect(entityId);
        AZ::TransformNotificationBus::Handler::BusConnect(entityId);
        AzToolsFramework::BoxManipulatorRequestBus::Handler::BusConnect(AZ::EntityComponentIdPair(entityId, componentId));
        AzToolsFramework::ShapeManipulatorRequestBus::Handler::BusConnect(AZ::EntityComponentIdPair(entityId, componentId));
        ColliderShapeRequestBus::Handler::BusConnect(entityId);
        EditorColliderComponentRequestBus::Handler::BusConnect(AZ::EntityComponentIdPair(entityId, componentId));
        EditorSimpleColliderComponentRequestBus::Handler::BusConnect(AZ::EntityComponentIdPair(entityId, componentId));
        AzFramework::BoundsRequestBus::Handler::BusConnect(entityId);
        AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusConnect(entityId);
        m_nonUniformScaleChangedHandler = AZ::NonUniformScaleChangedEvent::Handler(
            [this](const AZ::Vector3& scale) {OnNonUniformScaleChanged(scale); });
        AZ::NonUniformScaleRequestBus::Event(
            entityId, &AZ::NonUniformScaleRequests::RegisterScaleChangedEvent,
            m_nonUniformScaleChangedHandler);
        m_hasNonUniformScale = (AZ::NonUniformScaleRequestBus::FindFirstHandler(entityId) != nullptr);
        m_proxyShapeConfiguration.m_hasNonUniformScale = m_hasNonUniformScale;

        AZ::TransformBus::EventResult(m_cachedWorldTransform, entityId, &AZ::TransformInterface::GetWorldTM);
        m_cachedNonUniformScale = AZ::Vector3::CreateOne();
        if (m_hasNonUniformScale)
        {
            AZ::NonUniformScaleRequestBus::EventResult(m_cachedNonUniformScale, entityId, &AZ::NonUniformScaleRequests::GetScale);
        }

        // Debug drawing
        // m_colliderDebugDraw.Connect(entityId);
        // m_colliderDebugDraw.SetDisplayCallback(this);

        // ComponentMode
        m_componentModeDelegate.ConnectWithSingleComponentMode<
            EditorSimpleShapeColliderComponent, ColliderComponentMode>(
            AZ::EntityComponentIdPair(entityId, componentId), nullptr);

        UpdateCollider();
    }

    void EditorSimpleShapeColliderComponent::Deactivate()
    {
        AzPhysics::SimulatedBodyComponentRequestsBus::Handler::BusDisconnect();
        // m_colliderDebugDraw.Disconnect();
        m_nonUniformScaleChangedHandler.Disconnect();
        AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusDisconnect();
        AzFramework::BoundsRequestBus::Handler::BusDisconnect();
        EditorSimpleColliderComponentRequestBus::Handler::BusDisconnect();
        EditorColliderComponentRequestBus::Handler::BusDisconnect();
        ColliderShapeRequestBus::Handler::BusDisconnect();
        AzToolsFramework::ShapeManipulatorRequestBus::Handler::BusDisconnect();
        AzToolsFramework::BoxManipulatorRequestBus::Handler::BusDisconnect();
        AZ::TransformNotificationBus::Handler::BusDisconnect();
        AzToolsFramework::EntitySelectionEvents::Bus::Handler::BusDisconnect();
        AzToolsFramework::Components::EditorComponentBase::Deactivate();

        m_componentModeDelegate.Disconnect();

        // When Deactivate is triggered from an application shutdown, it's possible that the
        // scene interface has already been deleted, so check for its existence here again
        m_sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
        if (m_sceneInterface)
        {
            m_sceneInterface->RemoveSimulatedBody(m_editorSceneHandle, m_editorBodyHandle);
        }
    }

    AZ::u32 EditorSimpleShapeColliderComponent::OnConfigurationChanged()
    {
        m_configuration.m_materialSlots.SetSlots(Physics::MaterialDefaultSlot::Default); // Non-asset configs only have the default slot.
        m_configuration.m_materialSlots.SetSlotsReadOnly(false);

        // ensure we refresh the ComponentMode (and Manipulators) when the configuration
        // changes to keep the ComponentMode in sync with the shape (otherwise the manipulators
        // will move out of alignment with the shape)
        AzToolsFramework::ComponentModeFramework::ComponentModeSystemRequestBus::Broadcast(
            &AzToolsFramework::ComponentModeFramework::ComponentModeSystemRequests::Refresh,
            AZ::EntityComponentIdPair(GetEntityId(), GetId()));

        UpdateCollider();

        return AZ::Edit::PropertyRefreshLevels::None;
    }

    void EditorSimpleShapeColliderComponent::OnSelected()
    {
        if (auto* joltSystem = GetJoltSystem())
        {
            joltSystem->RegisterSystemConfigurationChangedEvent(m_joltConfigChangedHandler);
        }
    }

    void EditorSimpleShapeColliderComponent::OnDeselected()
    {
        m_joltConfigChangedHandler.Disconnect();
    }

    void EditorSimpleShapeColliderComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto sharedColliderConfig = AZStd::make_shared<Physics::ColliderConfiguration>(m_configuration);
        BaseColliderComponent* colliderComponent = nullptr;

        auto buildGameEntityScaledPrimitive = [gameEntity](AZStd::shared_ptr<Physics::ColliderConfiguration>& colliderConfig,
            Physics::ShapeConfiguration& shapeConfig, AZ::u8 subdivisionLevel)
        {
            auto scaledPrimitiveConfig = Utils::CreateConvexFromPrimitive(*colliderConfig,
                shapeConfig, subdivisionLevel, shapeConfig.m_scale);
            if (scaledPrimitiveConfig.has_value())
            {
                colliderConfig->m_rotation = AZ::Quaternion::CreateIdentity();
                colliderConfig->m_position = AZ::Vector3::CreateZero();
                BaseColliderComponent* colliderComponent = gameEntity->CreateComponent<BaseColliderComponent>();
                colliderComponent->SetShapeConfigurationList({ AZStd::make_pair(colliderConfig,
                    AZStd::make_shared<Physics::CookedMeshShapeConfiguration>(scaledPrimitiveConfig.value())) });
            }
        };

        switch (m_proxyShapeConfiguration.m_shapeType)
        {
        case Physics::ShapeType::Sphere:
            if (!m_hasNonUniformScale)
            {
                colliderComponent = gameEntity->CreateComponent<SphereColliderComponent>();
                colliderComponent->SetShapeConfigurationList({ AZStd::make_pair(sharedColliderConfig,
                    AZStd::make_shared<Physics::SphereShapeConfiguration>(m_proxyShapeConfiguration.m_sphere)) });
            }
            else
            {
                buildGameEntityScaledPrimitive(sharedColliderConfig, m_proxyShapeConfiguration.m_sphere, m_proxyShapeConfiguration.m_subdivisionLevel);
            }
            break;
        case Physics::ShapeType::Box:
            if (!m_hasNonUniformScale)
            {
                colliderComponent = gameEntity->CreateComponent<BoxColliderComponent>();
                colliderComponent->SetShapeConfigurationList({ AZStd::make_pair(sharedColliderConfig,
                    AZStd::make_shared<Physics::BoxShapeConfiguration>(m_proxyShapeConfiguration.m_box)) });
            }
            else
            {
                buildGameEntityScaledPrimitive(sharedColliderConfig, m_proxyShapeConfiguration.m_box, m_proxyShapeConfiguration.m_subdivisionLevel);
            }
            break;
        case Physics::ShapeType::Capsule:
            if (!m_hasNonUniformScale)
            {
                colliderComponent = gameEntity->CreateComponent<CapsuleColliderComponent>();
                colliderComponent->SetShapeConfigurationList({ AZStd::make_pair(sharedColliderConfig,
                    AZStd::make_shared<Physics::CapsuleShapeConfiguration>(m_proxyShapeConfiguration.m_capsule)) });
            }
            else
            {
                buildGameEntityScaledPrimitive(sharedColliderConfig, m_proxyShapeConfiguration.m_capsule, m_proxyShapeConfiguration.m_subdivisionLevel);
            }
            break;
        case Physics::ShapeType::Cylinder:
            UpdateCylinderCookedMesh();
            buildGameEntityScaledPrimitive(
                sharedColliderConfig, m_proxyShapeConfiguration.m_cylinder.m_configuration, m_proxyShapeConfiguration.m_subdivisionLevel);
            break;
        case Physics::ShapeType::CookedMesh:
            colliderComponent = gameEntity->CreateComponent<BaseColliderComponent>();
            colliderComponent->SetShapeConfigurationList({ AZStd::make_pair(sharedColliderConfig,
                AZStd::make_shared<Physics::CookedMeshShapeConfiguration>(m_proxyShapeConfiguration.m_cookedMesh)) });
            break;
        default:
            AZ_Warning("EditorColliderComponent", false, "Unsupported shape type for building game entity!");
            break;
        }
    }

    AZ::Transform EditorSimpleShapeColliderComponent::GetColliderLocalTransform() const
    {
        return AZ::Transform::CreateFromQuaternionAndTranslation(
            m_configuration.m_rotation, m_configuration.m_position);
    }

    void EditorSimpleShapeColliderComponent::UpdateCollider()
    {
        UpdateShapeConfiguration();
        CreateStaticEditorCollider();
        Physics::ColliderComponentEventBus::Event(GetEntityId(), &Physics::ColliderComponentEvents::OnColliderChanged);
    }

    void EditorSimpleShapeColliderComponent::CreateStaticEditorCollider()
    {
        m_cachedAabbDirty = true;

        if (!GetEntity()->FindComponent<EditorStaticRigidBodyComponent>())
        {
            m_colliderDebugDraw.ClearCachedGeometry();
            return;
        }

        AZ::Transform colliderTransform = GetWorldTM();
        colliderTransform.ExtractUniformScale();
        AzPhysics::StaticRigidBodyConfiguration configuration;
        configuration.m_orientation = colliderTransform.GetRotation();
        configuration.m_position = colliderTransform.GetTranslation();
        configuration.m_entityId = GetEntityId();
        configuration.m_debugName = GetEntity()->GetName();

        AZStd::shared_ptr<Physics::ColliderConfiguration> colliderConfig = AZStd::make_shared<Physics::ColliderConfiguration>(
            GetColliderConfigurationScaled());
        AZStd::shared_ptr<Physics::ShapeConfiguration> shapeConfig = m_proxyShapeConfiguration.CloneCurrent();

        if (m_proxyShapeConfiguration.IsNonUniformlyScaledPrimitive() || m_proxyShapeConfiguration.IsCylinderConfig())
        {
            auto convexConfig = Utils::CreateConvexFromPrimitive(GetColliderConfiguration(), *(shapeConfig.get()),
                m_proxyShapeConfiguration.m_subdivisionLevel, shapeConfig->m_scale);
            Physics::ColliderConfiguration colliderConfigurationNoOffset = *colliderConfig;
            colliderConfigurationNoOffset.m_rotation = AZ::Quaternion::CreateIdentity();
            colliderConfigurationNoOffset.m_position = AZ::Vector3::CreateZero();

            if (convexConfig.has_value())
            {
                AZStd::shared_ptr<Physics::Shape> shape = AZ::Interface<Physics::System>::Get()->CreateShape(
                    colliderConfigurationNoOffset, convexConfig.value());
                configuration.m_colliderAndShapeData = shape;
            }
        }
        else
        {
            configuration.m_colliderAndShapeData = AzPhysics::ShapeColliderPair(colliderConfig, shapeConfig);
        }

        if (m_sceneInterface)
        {
            //remove the previous body if any
            if (m_editorBodyHandle != AzPhysics::InvalidSimulatedBodyHandle)
            {
                m_sceneInterface->RemoveSimulatedBody(m_editorSceneHandle, m_editorBodyHandle);
            }
            
            m_editorBodyHandle = m_sceneInterface->AddSimulatedBody(m_editorSceneHandle, &configuration);
        }

        m_colliderDebugDraw.ClearCachedGeometry();

        AzPhysics::SimulatedBodyComponentRequestsBus::Handler::BusConnect(GetEntityId());
    }

    void EditorSimpleShapeColliderComponent::BuildDebugDrawMesh() const
    {
        const AZ::u32 shapeIndex = 0; // There's only one mesh gets built from the primitive collider, hence use geomIndex 0.
        if (m_proxyShapeConfiguration.IsCylinderConfig())
        {
            physx::PxGeometryHolder pxGeometryHolder;
            Utils::CreatePxGeometryFromConfig(
                m_proxyShapeConfiguration.m_cylinder.m_configuration, pxGeometryHolder); // this will cause the native mesh to be cached

            m_colliderDebugDraw.BuildMeshes(m_proxyShapeConfiguration.m_cylinder.m_configuration, shapeIndex);
        }
        else if (!m_hasNonUniformScale)
        {
            m_colliderDebugDraw.BuildMeshes(m_proxyShapeConfiguration.GetCurrent(), shapeIndex);
        }
        else
        {
            m_scaledPrimitive = Utils::CreateConvexFromPrimitive(GetColliderConfiguration(), m_proxyShapeConfiguration.GetCurrent(),
                m_proxyShapeConfiguration.m_subdivisionLevel, m_proxyShapeConfiguration.GetCurrent().m_scale);
            if (m_scaledPrimitive.has_value())
            {
                physx::PxGeometryHolder pxGeometryHolder;
                Utils::CreatePxGeometryFromConfig(m_scaledPrimitive.value(), pxGeometryHolder); // this will cause the native mesh to be cached
                m_colliderDebugDraw.BuildMeshes(m_scaledPrimitive.value(), shapeIndex);
            }
        }
    }

    void EditorSimpleShapeColliderComponent::DisplayCylinderCollider(AzFramework::DebugDisplayRequests& debugDisplay) const
    {
        const AZ::u32 shapeIndex = 0;
        m_colliderDebugDraw.DrawMesh(
            debugDisplay,
            GetColliderConfigurationNoOffset(),
            m_proxyShapeConfiguration.m_cylinder.m_configuration,
            m_proxyShapeConfiguration.m_cylinder.m_configuration.m_scale,
            shapeIndex);
    }

    void EditorSimpleShapeColliderComponent::DisplayScaledPrimitiveCollider(AzFramework::DebugDisplayRequests& debugDisplay) const
    {
        if (m_scaledPrimitive.has_value())
        {
            const AZ::u32 shapeIndex = 0;
            Physics::ColliderConfiguration colliderConfigNoOffset = m_configuration;
            colliderConfigNoOffset.m_rotation = AZ::Quaternion::CreateIdentity();
            colliderConfigNoOffset.m_position = AZ::Vector3::CreateZero();
            m_colliderDebugDraw.DrawMesh(debugDisplay, colliderConfigNoOffset, m_scaledPrimitive.value(),
                GetWorldTM().GetUniformScale() * m_cachedNonUniformScale, shapeIndex);
        }
    }

    void EditorSimpleShapeColliderComponent::DisplayUnscaledPrimitiveCollider(AzFramework::DebugDisplayRequests& debugDisplay) const
    {
        switch (m_proxyShapeConfiguration.m_shapeType)
        {
        case Physics::ShapeType::Sphere:
            m_colliderDebugDraw.DrawSphere(debugDisplay, m_configuration, m_proxyShapeConfiguration.m_sphere);
            break;
        case Physics::ShapeType::Box:
            m_colliderDebugDraw.DrawBox(debugDisplay, m_configuration, m_proxyShapeConfiguration.m_box);
            break;
        case Physics::ShapeType::Capsule:
            m_colliderDebugDraw.DrawCapsule(debugDisplay, m_configuration, m_proxyShapeConfiguration.m_capsule);
            break;
        }
    }

    void EditorSimpleShapeColliderComponent::Display([[maybe_unused]] const AzFramework::ViewportInfo& viewportInfo,
        AzFramework::DebugDisplayRequests& debugDisplay) const
    {
        if (!m_colliderDebugDraw.HasCachedGeometry())
        {
            BuildDebugDrawMesh();
        }

        if (m_colliderDebugDraw.HasCachedGeometry())
        {
            if (m_proxyShapeConfiguration.IsCylinderConfig())
            {
                DisplayCylinderCollider(debugDisplay);
            }
            else 
            {
                if (m_hasNonUniformScale)
                {
                    DisplayScaledPrimitiveCollider(debugDisplay);
                }
                else
                {
                    DisplayUnscaledPrimitiveCollider(debugDisplay);
                }
            }
        }
    }

    AZ::Vector3 EditorSimpleShapeColliderComponent::GetDimensions() const
    {
        return GetBoxDimensions();
    }

    void EditorSimpleShapeColliderComponent::SetDimensions(const AZ::Vector3& dimensions)
    {
        SetBoxDimensions(dimensions);
    }

    AZ::Vector3 EditorSimpleShapeColliderComponent::GetTranslationOffset() const
    {
        return m_configuration.m_position;
    }

    void EditorSimpleShapeColliderComponent::SetTranslationOffset(const AZ::Vector3& translationOffset)
    {
        m_configuration.m_position = translationOffset;
        UpdateCollider();
    }

    AZ::Transform EditorSimpleShapeColliderComponent::GetCurrentLocalTransform() const
    {
        return GetColliderLocalTransform();
    }

    AZ::Transform EditorSimpleShapeColliderComponent::GetManipulatorSpace() const
    {
        return GetWorldTM();
    }

    AZ::Quaternion EditorSimpleShapeColliderComponent::GetRotationOffset() const
    {
        return m_configuration.m_rotation;
    }

    void EditorSimpleShapeColliderComponent::OnTransformChanged(const AZ::Transform& /*local*/, const AZ::Transform& world)
    {
        if (world.IsClose(m_cachedWorldTransform))
        {
            return;
        }
        m_cachedWorldTransform = world;

        UpdateCollider();
    }

    void EditorSimpleShapeColliderComponent::OnNonUniformScaleChanged(const AZ::Vector3& nonUniformScale)
    {
        m_cachedNonUniformScale = nonUniformScale;

        UpdateCollider();
    }

    // JoltPhysics::ColliderShapeBus
    AZ::Aabb EditorSimpleShapeColliderComponent::GetColliderShapeAabb()
    {
        if (m_cachedAabbDirty)
        {
            m_cachedAabb = JoltPhysics::Utils::GetColliderAabb(GetWorldTM()
                , m_hasNonUniformScale
                , m_proxyShapeConfiguration.m_subdivisionLevel
                , m_proxyShapeConfiguration.GetCurrent()
                , m_configuration);
            m_cachedAabbDirty = false;
        }

        return m_cachedAabb;
    }

    void EditorSimpleShapeColliderComponent::UpdateShapeConfigurationScale()
    {
        auto& shapeConfiguration = m_proxyShapeConfiguration.GetCurrent();
        shapeConfiguration.m_scale = GetWorldTM().ExtractUniformScale() * m_cachedNonUniformScale;
    }

    void EditorSimpleShapeColliderComponent::EnablePhysics()
    {
        if (!IsPhysicsEnabled())
        {
            UpdateCollider();
        }
    }

    void EditorSimpleShapeColliderComponent::DisablePhysics()
    {
        if (m_sceneInterface && m_editorBodyHandle != AzPhysics::InvalidSimulatedBodyHandle)
        {
            m_sceneInterface->RemoveSimulatedBody(m_editorSceneHandle, m_editorBodyHandle);
        }
    }

    bool EditorSimpleShapeColliderComponent::IsPhysicsEnabled() const
    {
        if (m_sceneInterface && m_editorBodyHandle != AzPhysics::InvalidSimulatedBodyHandle)
        {
            if (auto* body = m_sceneInterface->GetSimulatedBodyFromHandle(m_editorSceneHandle, m_editorBodyHandle))
            {
                return body->m_simulating;
            }
        }
        return false;
    }

    AZ::Aabb EditorSimpleShapeColliderComponent::GetAabb() const
    {
        if (m_sceneInterface && m_editorBodyHandle != AzPhysics::InvalidSimulatedBodyHandle)
        {
            if (auto* body = m_sceneInterface->GetSimulatedBodyFromHandle(m_editorSceneHandle, m_editorBodyHandle))
            {
                return body->GetAabb();
            }
        }
        return AZ::Aabb::CreateNull();
    }

    AzPhysics::SimulatedBody* EditorSimpleShapeColliderComponent::GetSimulatedBody()
    {
        if (m_sceneInterface && m_editorBodyHandle != AzPhysics::InvalidSimulatedBodyHandle)
        {
            if (auto* body = m_sceneInterface->GetSimulatedBodyFromHandle(m_editorSceneHandle, m_editorBodyHandle))
            {
                return body;
            }
        }
        return nullptr;
    }

    AzPhysics::SimulatedBodyHandle EditorSimpleShapeColliderComponent::GetSimulatedBodyHandle() const
    {
        return m_editorBodyHandle;
    }

    AzPhysics::SceneQueryHit EditorSimpleShapeColliderComponent::RayCast(const AzPhysics::RayCastRequest& request)
    {
        if (m_sceneInterface && m_editorBodyHandle != AzPhysics::InvalidSimulatedBodyHandle)
        {
            if (auto* body = m_sceneInterface->GetSimulatedBodyFromHandle(m_editorSceneHandle, m_editorBodyHandle))
            {
                return body->RayCast(request);
            }
        }
        return AzPhysics::SceneQueryHit();
    }

    bool EditorSimpleShapeColliderComponent::IsTrigger()
    {
        return m_configuration.m_isTrigger;
    }

    void EditorSimpleShapeColliderComponent::SetColliderOffset(const AZ::Vector3& offset)
    {
        m_configuration.m_position = offset;
        UpdateCollider();
    }

    AZ::Vector3 EditorSimpleShapeColliderComponent::GetColliderOffset() const
    {
        return m_configuration.m_position;
    }

    void EditorSimpleShapeColliderComponent::SetColliderRotation(const AZ::Quaternion& rotation)
    {
        m_configuration.m_rotation = rotation;
        UpdateCollider();
    }

    AZ::Quaternion EditorSimpleShapeColliderComponent::GetColliderRotation() const
    {
        return m_configuration.m_rotation;
    }

    AZ::Transform EditorSimpleShapeColliderComponent::GetColliderWorldTransform() const
    {
        return GetWorldTM() * GetColliderLocalTransform();
    }

    void EditorSimpleShapeColliderComponent::SetShapeType(Physics::ShapeType shapeType)
    {
        m_proxyShapeConfiguration.m_shapeType = shapeType;

        if (shapeType == Physics::ShapeType::Cylinder)
        {
            UpdateCylinderCookedMesh();
        }

        UpdateCollider();
    }

    Physics::ShapeType EditorSimpleShapeColliderComponent::GetShapeType() const
    {
        return m_proxyShapeConfiguration.m_shapeType;
    }

    void EditorSimpleShapeColliderComponent::SetBoxDimensions(const AZ::Vector3& dimensions)
    {
        m_proxyShapeConfiguration.m_box.m_dimensions = dimensions;
        UpdateCollider();
    }

    AZ::Vector3 EditorSimpleShapeColliderComponent::GetBoxDimensions() const
    {
        return m_proxyShapeConfiguration.m_box.m_dimensions;
    }

    void EditorSimpleShapeColliderComponent::SetSphereRadius(float radius)
    {
        m_proxyShapeConfiguration.m_sphere.m_radius = radius;
        UpdateCollider();
    }

    float EditorSimpleShapeColliderComponent::GetSphereRadius() const
    {
        return m_proxyShapeConfiguration.m_sphere.m_radius;
    }

    void EditorSimpleShapeColliderComponent::SetCapsuleRadius(float radius)
    {
        m_proxyShapeConfiguration.m_capsule.m_radius = radius;
        UpdateCollider();
    }

    float EditorSimpleShapeColliderComponent::GetCapsuleRadius() const
    {
        return m_proxyShapeConfiguration.m_capsule.m_radius;
    }

    void EditorSimpleShapeColliderComponent::SetCapsuleHeight(float height)
    {
        m_proxyShapeConfiguration.m_capsule.m_height = height;
        UpdateCollider();
    }

    float EditorSimpleShapeColliderComponent::GetCapsuleHeight() const
    {
        return m_proxyShapeConfiguration.m_capsule.m_height;
    }

    void EditorSimpleShapeColliderComponent::SetCylinderRadius(float radius)
    {
        if (radius <= 0.0f)
        {
            AZ_Error("Jolt", false, "SetCylinderRadius: radius must be greater than zero.");
            return;
        }

        m_proxyShapeConfiguration.m_cylinder.m_radius = radius;
        UpdateCylinderCookedMesh();
        UpdateCollider();
    }

    float EditorSimpleShapeColliderComponent::GetCylinderRadius() const
    {
        return m_proxyShapeConfiguration.m_cylinder.m_radius;
    }

    void EditorSimpleShapeColliderComponent::SetCylinderHeight(float height)
    {
        if (height <= 0.0f)
        {
            AZ_Error("Jolt", false, "SetCylinderHeight: height must be greater than zero.");
            return;
        }

        m_proxyShapeConfiguration.m_cylinder.m_height = height;
        UpdateCylinderCookedMesh();
        UpdateCollider();
    }

    float EditorSimpleShapeColliderComponent::GetCylinderHeight() const
    {
        return m_proxyShapeConfiguration.m_cylinder.m_height;
    }

    void EditorSimpleShapeColliderComponent::SetCylinderSubdivisionCount(AZ::u8 subdivisionCount)
    {
        const AZ::u8 clampedSubdivisionCount = AZ::GetClamp(subdivisionCount, Utils::MinFrustumSubdivisions, Utils::MaxFrustumSubdivisions);
        AZ_Warning(
            "Jolt",
            clampedSubdivisionCount == subdivisionCount,
            "Requested cylinder subdivision count %d clamped into allowed range (%d - %d). Entity: %s",
            subdivisionCount,
            Utils::MinFrustumSubdivisions,
            Utils::MaxFrustumSubdivisions,
            GetEntity()->GetName().c_str());
        m_proxyShapeConfiguration.m_cylinder.m_subdivisionCount = clampedSubdivisionCount;
        UpdateCylinderCookedMesh();
        UpdateCollider();
    }

    AZ::u8 EditorSimpleShapeColliderComponent::GetCylinderSubdivisionCount() const
    {
        return m_proxyShapeConfiguration.m_cylinder.m_subdivisionCount;
    }

    void EditorSimpleShapeColliderComponent::UpdateShapeConfiguration()
    {
        UpdateShapeConfigurationScale();

        if (m_proxyShapeConfiguration.IsCylinderConfig())
        {
            // Create cooked cylinder convex
            UpdateCylinderCookedMesh();
        }
    }

    void EditorSimpleShapeColliderComponent::UpdateCylinderCookedMesh()
    {
        const AZ::u8 subdivisionCount = m_proxyShapeConfiguration.m_cylinder.m_subdivisionCount;
        const float height = m_proxyShapeConfiguration.m_cylinder.m_height;
        const float radius = m_proxyShapeConfiguration.m_cylinder.m_radius;

        if (height <= 0.0f)
        {
            AZ_Error("Jolt", false, "Cylinder height must be greater than zero. Entity: %s", GetEntity()->GetName().c_str());
            return;
        }

        if (radius <= 0.0f)
        {
            AZ_Error("Jolt", false, "Cylinder radius must be greater than zero. Entity: %s", GetEntity()->GetName().c_str());
            return;
        }

        Utils::Geometry::PointList samplePoints = Utils::CreatePointsAtFrustumExtents(height, radius, radius, subdivisionCount).value();

        const AZ::Transform colliderLocalTransform = GetColliderLocalTransform();

        AZStd::transform(
            samplePoints.begin(),
            samplePoints.end(),
            samplePoints.begin(),
            [&colliderLocalTransform](const AZ::Vector3& point)
            {
                return colliderLocalTransform.TransformPoint(point);
            });

        const AZ::Vector3 scale = m_proxyShapeConfiguration.m_cylinder.m_configuration.m_scale;
        m_proxyShapeConfiguration.m_cylinder.m_configuration = Utils::CreateJoltCookedMeshConfiguration(samplePoints, scale).value();
    }

    AZ::Aabb EditorSimpleShapeColliderComponent::GetWorldBounds() const
    {
        return GetAabb();
    }

    AZ::Aabb EditorSimpleShapeColliderComponent::GetLocalBounds() const
    {
        AZ::Aabb worldBounds = GetWorldBounds();
        if (worldBounds.IsValid())
        {
            return worldBounds.GetTransformedAabb(m_cachedWorldTransform.GetInverse());
        }

        return AZ::Aabb::CreateNull();
    }

    bool EditorSimpleShapeColliderComponent::SupportsEditorRayIntersect()
    {
        return true;
    }

    AZ::Aabb EditorSimpleShapeColliderComponent::GetEditorSelectionBoundsViewport([[maybe_unused]] const AzFramework::ViewportInfo& viewportInfo)
    {
        return GetWorldBounds();
    }

    bool EditorSimpleShapeColliderComponent::EditorSelectionIntersectRayViewport(
        [[maybe_unused]] const AzFramework::ViewportInfo& viewportInfo, const AZ::Vector3& src, const AZ::Vector3& dir, float& distance)
    {
        AzPhysics::RayCastRequest request;
        request.m_direction = dir;
        request.m_distance = distance;
        request.m_start = src;

        if (auto hit = RayCast(request))
        {
            distance = hit.m_distance;
            return true;
        }

        return false;
    }
} // JoltPhysics
