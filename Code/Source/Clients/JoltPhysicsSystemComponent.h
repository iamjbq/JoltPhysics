
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

#include <JoltPhysics/JoltPhysicsBus.h>

namespace JoltPhysics
{
    class JoltSystem;

    class JoltPhysicsSystemComponent
        : public AZ::Component
        , protected JoltPhysicsRequestBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(JoltPhysicsSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        JoltPhysicsSystemComponent();
        ~JoltPhysicsSystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // JoltPhysicsRequestBus interface implementation
        
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZTickBus interface implementation
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        int GetTickOrder() override;
        ////////////////////////////////////////////////////////////////////////

    private:
        JoltSystem* m_joltSystem;
        bool m_isTickingPhysics = false;
        // AzPhysics::SystemEvents::OnInitializedEvent::Handler m_onSystemInitializedHandler;
        // AzPhysics::SystemEvents::OnConfigurationChangedEvent::Handler m_onSystemConfigChangedHandler;

        // Not sure if this is needed for Jolt implementation, but maybe a good switch to have
        void EnableAutoManagedPhysicsTick(bool shouldTick);

        void ActivateSimulation();
    };

} // namespace JoltPhysics
