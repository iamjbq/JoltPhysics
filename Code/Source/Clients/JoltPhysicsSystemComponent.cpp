
#include <Clients/JoltPhysicsSystemComponent.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzFramework/Physics/Material/PhysicsMaterialAsset.h>

#include <Material/JoltMaterialManager.h>
#include <System/JoltSystem.h>
#include <Clients/Shape.h>
#include <JoltPhysics/Configuration/JoltConfiguration.h>
#include <JoltPhysics/JoltPhysicsTypeIds.h>

namespace JoltPhysics
{
    AZ_COMPONENT_IMPL(JoltPhysicsSystemComponent, "JoltPhysicsSystemComponent", JoltPhysicsSystemComponentTypeId);

    void JoltPhysicsSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        // These were bundled in void ReflectPhysXOnlyApi(AZ::ReflectContext* context) in PhysX
        JoltSystemConfiguration::Reflect(context);
        MaterialConfiguration::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<JoltPhysicsSystemComponent, AZ::Component>()
                ->Version(1)
                ->Attribute(AZ::Edit::Attributes::SystemComponentTags, AZStd::vector<AZ::Crc32>({ AZ_CRC_CE("AssetBuilder") }))
                ->Field("Enabled", &JoltPhysicsSystemComponent::m_enabled)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<JoltPhysicsSystemComponent>("Jolt", "Global Jolt physics configuration.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Jolt")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &JoltPhysicsSystemComponent::m_enabled,
                    "Enabled", "Enables the Jolt system component.")
                ;
            }
        }
    }

    void JoltPhysicsSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("PhysicsService"));
    }

    void JoltPhysicsSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("PhysicsService"));
    }

    void JoltPhysicsSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void JoltPhysicsSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("AssetDatabaseService"));
        dependent.push_back(AZ_CRC_CE("AssetCatalogService"));
    }

    JoltPhysicsSystemComponent::JoltPhysicsSystemComponent()
        : m_enabled(true)
        , m_onSystemInitializedHandler(
            [this](const AzPhysics::SystemConfiguration* config)
            {
                EnableAutoManagedPhysicsTick(config->m_autoManageSimulationUpdate);
            })
        , m_onSystemConfigChangedHandler(
            [this](const AzPhysics::SystemConfiguration* config)
            {
                EnableAutoManagedPhysicsTick(config->m_autoManageSimulationUpdate);
            })
    {
    }

    JoltPhysicsSystemComponent::~JoltPhysicsSystemComponent()
    {
        if (m_physicsSystem.Get() == this)
        {
            m_physicsSystem.Unregister(this);
        }
    }

    AZStd::shared_ptr<Physics::Shape> JoltPhysicsSystemComponent::CreateShape(
        const Physics::ColliderConfiguration& colliderConfiguration, const Physics::ShapeConfiguration& configuration)
    {
        auto shapePtr = AZStd::make_shared<JoltPhysics::Shape>(colliderConfiguration, configuration);

        if (shapePtr->GetNativePointer())
        {
            return shapePtr;
        }

        AZ_Error("Jolt", false, "SystemComponent::CreateShape error. Unable to create a shape from configuration.");

        return nullptr;
    }

    void JoltPhysicsSystemComponent::ReleaseNativeMeshObject([[maybe_unused]] void* nativeMeshObject)
    {
    }

    void JoltPhysicsSystemComponent::ReleaseNativeHeightfieldObject([[maybe_unused]] void* nativeHeightfieldObject)
    {
    }

    bool JoltPhysicsSystemComponent::CookConvexMeshToFile([[maybe_unused]] const AZStd::string& filePath, [[maybe_unused]] const AZ::Vector3* vertices,
        [[maybe_unused]] AZ::u32 vertexCount)
    {
        return false;
    }

    bool JoltPhysicsSystemComponent::CookConvexMeshToMemory([[maybe_unused]] const AZ::Vector3* vertices, [[maybe_unused]] AZ::u32 vertexCount,
        [[maybe_unused]] AZStd::vector<AZ::u8>& result)
    {
        return false;
    }

    bool JoltPhysicsSystemComponent::CookTriangleMeshToFile([[maybe_unused]] const AZStd::string& filePath, [[maybe_unused]] const AZ::Vector3* vertices,
        [[maybe_unused]] AZ::u32 vertexCount, [[maybe_unused]] const AZ::u32* indices, [[maybe_unused]] AZ::u32 indexCount)
    {
        return false;
    }

    bool JoltPhysicsSystemComponent::CookTriangleMeshToMemory([[maybe_unused]] const AZ::Vector3* vertices, [[maybe_unused]] AZ::u32 vertexCount,
        [[maybe_unused]] const AZ::u32* indices, [[maybe_unused]] AZ::u32 indexCount, [[maybe_unused]] AZStd::vector<AZ::u8>& result)
    {
        return false;
    }

    AzPhysics::CollisionLayer JoltPhysicsSystemComponent::GetCollisionLayerByName(const AZStd::string& layerName)
    {
        return m_joltSystem->GetJoltConfiguration().m_collisionConfig.m_collisionLayers.GetLayer(layerName);
    }

    AZStd::string JoltPhysicsSystemComponent::GetCollisionLayerName(const AzPhysics::CollisionLayer& layer)
    {
        return m_joltSystem->GetJoltConfiguration().m_collisionConfig.m_collisionLayers.GetName(layer);
    }

    bool JoltPhysicsSystemComponent::TryGetCollisionLayerByName(const AZStd::string& layerName,
        AzPhysics::CollisionLayer& layer)
    {
        return m_joltSystem->GetJoltConfiguration().m_collisionConfig.m_collisionLayers.TryGetLayer(layerName, layer);
    }

    AzPhysics::CollisionGroup JoltPhysicsSystemComponent::GetCollisionGroupByName(const AZStd::string& groupName)
    {
        return m_joltSystem->GetJoltConfiguration().m_collisionConfig.m_collisionGroups.FindGroupByName(groupName);
    }

    bool JoltPhysicsSystemComponent::TryGetCollisionGroupByName(const AZStd::string& layerName,
        AzPhysics::CollisionGroup& group)
    {
        return m_joltSystem->GetJoltConfiguration().m_collisionConfig.m_collisionGroups.TryFindGroupByName(layerName, group);
    }

    AZStd::string JoltPhysicsSystemComponent::GetCollisionGroupName(const AzPhysics::CollisionGroup& collisionGroup)
    {
        AZStd::string groupName;
        for (const auto& group : m_joltSystem->GetJoltConfiguration().m_collisionConfig.m_collisionGroups.GetPresets())
        {
            if (group.m_group.GetMask() == collisionGroup.GetMask())
            {
                groupName = group.m_name;
                break;
            }
        }
        return groupName;
    }

    AzPhysics::CollisionGroup JoltPhysicsSystemComponent::GetCollisionGroupById(
        const AzPhysics::CollisionGroups::Id& groupId)
    {
        auto groups = m_joltSystem->GetJoltConfiguration().m_collisionConfig.m_collisionGroups;
        return groups.FindGroupById(groupId);
    }

    void JoltPhysicsSystemComponent::SetCollisionLayerName(int index, const AZStd::string& layerName)
    {
        m_joltSystem->SetCollisionLayerName(index, layerName);
    }

    void JoltPhysicsSystemComponent::CreateCollisionGroup(const AZStd::string& groupName,
        const AzPhysics::CollisionGroup& group)
    {
        m_joltSystem->CreateCollisionGroup(groupName, group);
    }

    bool JoltPhysicsSystemComponent::ShouldCollide([[maybe_unused]] const Physics::ColliderConfiguration& colliderConfigurationA,
        [[maybe_unused]] const Physics::ColliderConfiguration& colliderConfigurationB)
    {
        AZ_Info("JoltPhysicsSystemComponent::ShouldCollide","Not implemented")
        return false;
    }

    void JoltPhysicsSystemComponent::Init()
    {
        if (m_physicsSystem.Get() == nullptr)
        {
            m_physicsSystem.Register(this);
        }
    }

    template<typename AssetHandlerT, typename AssetT>
    void RegisterAsset(AZStd::vector<AZStd::unique_ptr<AZ::Data::AssetHandler>>& assetHandlers)
    {
        AssetHandlerT* handler = aznew AssetHandlerT();
        AZ::Data::AssetCatalogRequestBus::Broadcast(&AZ::Data::AssetCatalogRequests::EnableCatalogForAsset, AZ::AzTypeInfo<AssetT>::Uuid());
        AZ::Data::AssetCatalogRequestBus::Broadcast(&AZ::Data::AssetCatalogRequests::AddExtension, AssetHandlerT::s_assetFileExtension);
        assetHandlers.emplace_back(handler);
    }

    void JoltPhysicsSystemComponent::Activate()
    {
        if (!m_enabled)
        {
            return;
        }

        m_defaultWorldComponent.Activate();

        Physics::SystemRequestBus::Handler::BusConnect();
        // JoltPhysicsRequestBus::Handler::BusConnect();
        Physics::CollisionRequestBus::Handler::BusConnect();

        ActivateSimulation();
    }

    void JoltPhysicsSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        Physics::CollisionRequestBus::Handler::BusDisconnect();
        // JoltPhysicsRequestBus::Handler::BusDisconnect();
        Physics::SystemRequestBus::Handler::BusDisconnect();

        m_defaultWorldComponent.Deactivate();

        m_materialManager.reset();

        m_onSystemInitializedHandler.Disconnect();
        m_onSystemConfigChangedHandler.Disconnect();

        if (m_joltSystem != nullptr)
        {
            m_joltSystem->Shutdown();
            m_joltSystem = nullptr;
        }
    }

    void JoltPhysicsSystemComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_joltSystem)
        {
            m_joltSystem->Simulate(deltaTime);
        }
    }

    int JoltPhysicsSystemComponent::GetTickOrder()
    {
        return AZ::ComponentTickBus::TICK_PHYSICS_SYSTEM;
    }

    void JoltPhysicsSystemComponent::EnableAutoManagedPhysicsTick(bool shouldTick)
    {
        if (shouldTick && !m_isTickingPhysics)
        {
            AZ::TickBus::Handler::BusConnect();
        }
        else if (!shouldTick && m_isTickingPhysics)
        {
            AZ::TickBus::Handler::BusDisconnect();
        }
        m_isTickingPhysics = shouldTick;
    }

    void JoltPhysicsSystemComponent::ActivateSimulation()
    {
        m_joltSystem = GetJoltSystem();

        if (m_joltSystem)
        {
            // Basic default config for testing
            // TODO: implement settings registry manager
            m_joltSystem->RegisterSystemInitializedEvent(m_onSystemInitializedHandler);
            m_joltSystem->RegisterSystemConfigurationChangedEvent(m_onSystemConfigChangedHandler);

            const JoltSystemConfiguration defaultConfig = JoltSystemConfiguration::CreateDefault();
            m_joltSystem->Initialize(&defaultConfig);

            // Default scene configuration for testing without settings
            const AzPhysics::SceneConfiguration defaultSceneConfig = AzPhysics::SceneConfiguration::CreateDefault();
            m_joltSystem->UpdateDefaultSceneConfiguration(defaultSceneConfig);

            AZ_Info("JoltSystemComponent", "Simulation activated")
        }

        AZ_Info("JoltSystemComponent", "JoltSystem was null")
        m_materialManager = AZStd::make_unique<MaterialManager>();
        m_materialManager->Init();
    }
} // namespace JoltPhysics
