
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/Interface/Interface.h>
#include <AzFramework/Physics/Character.h>
#include <AzFramework/Physics/SimulatedBodies/RigidBody.h>
#include <AzFramework/Physics/Shape.h>
#include <AzFramework/Physics/ShapeConfiguration.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/CollisionBus.h>
#include <AzFramework/Physics/Configuration/CollisionConfiguration.h>
#include <AzFramework/Physics/Collision/CollisionGroups.h>
#include <AzFramework/Physics/Collision/CollisionLayers.h>
#include <AzFramework/Physics/Common/PhysicsEvents.h>

#include <JoltPhysics/JoltPhysicsBus.h>

namespace JoltPhysics
{
    class JoltSystem;

    class JoltPhysicsSystemComponent
        : public AZ::Component
        // , public Physics::SystemRequestBus::Handler
        // , public JoltPhysics::SystemRequestsBus::Handler
        // , private Physics::CollisionRequestBus::Handler
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
        // Not sure if this is needed for Jolt implementation, but maybe a good switch to have
        void EnableAutoManagedPhysicsTick(bool shouldTick);

        void ActivateSimulation();

        bool m_enabled; ///< If false, this component will not activate itself in the Activate() function.

        // AZ::Interface<Physics::System> m_physicsSystem;

        JoltSystem* m_joltSystem = nullptr;
        bool m_isTickingPhysics = false;
        AzPhysics::SystemEvents::OnInitializedEvent::Handler m_onSystemInitializedHandler;
        AzPhysics::SystemEvents::OnConfigurationChangedEvent::Handler m_onSystemConfigChangedHandler;

    };

} // namespace JoltPhysics
