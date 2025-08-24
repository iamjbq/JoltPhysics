
#pragma once

#include <AzCore/Component/Component.h>

namespace JoltPhysics
{
    class EditorRigidBodyComponent
        : public AZ::Component
    {
    public:
        AZ_COMPONENT_DECL(EditorRigidBodyComponent);

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
