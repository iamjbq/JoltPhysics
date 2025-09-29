
#include "JoltPhysicsSystemComponent.h"

#include <iostream>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzFramework/Physics/Material/PhysicsMaterialAsset.h>

#include <System/JoltSystem.h>
#include <JoltPhysics/Configuration/JoltConfiguration.h>
#include <JoltPhysics/JoltPhysicsTypeIds.h>

namespace JoltPhysics
{
    // Disable common warnings triggered by Jolt, you can use JPH_SUPPRESS_WARNING_PUSH / JPH_SUPPRESS_WARNING_POP to store and restore the warning state
    JPH_SUPPRESS_WARNINGS

    // If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
    using namespace JPH::literals;
    
    // Callback for traces, connect this to your own trace function if you have one
    static void JoltTraceImpl(const char *inFMT, ...)
    {
        // Format the message
        va_list list;
        va_start(list, inFMT);
        // char buffer[1024];
        // vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);

        // Print to the console
        AZ_Trace("Jolt", inFMT, list)
    }
    
#ifdef JPH_ENABLE_ASSERTS
    
    // Callback for asserts, connect this to your own assert handler if you have one
    static bool JoltAssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, JPH::uint inLine)
    {
        // Print to the TTY
        std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << std::endl;
        // AZ_Error(inFile, inLine, inExpression, inMessage)

        // Breakpoint
        return true;
    };
    
#endif // JPH_ENABLE_ASSERTS
    
    AZ_COMPONENT_IMPL(JoltPhysicsSystemComponent, "JoltPhysicsSystemComponent", JoltPhysicsSystemComponentTypeId);

    void JoltPhysicsSystemComponent::Reflect(AZ::ReflectContext* context)
    {
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
        // if (m_physicsSystem.Get() == this)
        // {
        //     m_physicsSystem.Unregister(this);
        // }
    }

    void JoltPhysicsSystemComponent::Init()
    {
        // if (m_physicsSystem.Get() == nullptr)
        // {
        //     m_physicsSystem.Register(this);
        // }
        
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
        // JoltPhysicsRequestBus::Handler::BusConnect();

        ActivateSimulation();
    }

    void JoltPhysicsSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        // JoltPhysicsRequestBus::Handler::BusDisconnect();

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
    }
} // namespace JoltPhysics
