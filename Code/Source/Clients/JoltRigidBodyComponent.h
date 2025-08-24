
#pragma once

#include <AzCore/Component/Component.h>
#include <JoltPhysics/JoltRigidBodyInterface.h>

namespace JoltPhysics
{
    /*
    * TODO: Register this component in your Gem's AZ::Module interface by inserting the following into the list of m_descriptors:
    *       JoltRigidBodyComponent::CreateDescriptor(),
    */

    class JoltRigidBodyComponent
        : public AZ::Component
        , public JoltRigidBodyRequestBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(JoltRigidBodyComponent);

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        void Activate() override;
        void Deactivate() override;
    };
} // namespace JoltPhysics
