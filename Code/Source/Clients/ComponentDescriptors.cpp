
#include <Clients/ComponentDescriptors.h>

#include <Clients/JoltPhysicsSystemComponent.h>
#include <Clients/BaseColliderComponent.h>
// #include <Clients/BoxColliderComponent.h>
// #include <Clients/CapsuleColliderComponent.h>
// #include <Clients/SphereColliderComponent.h>
#include <Clients/ShapeColliderComponent.h>
#include <Clients/RigidBodyComponent.h>
#include <Clients/StaticRigidBodyComponent.h>

namespace JoltPhysics
{
    AZStd::list<AZ::ComponentDescriptor*> GetDescriptors()
    {
        AZStd::list<AZ::ComponentDescriptor*> descriptors =
        {
            JoltPhysicsSystemComponent::CreateDescriptor(),
            BaseColliderComponent::CreateDescriptor(),
            // BoxColliderComponent::CreateDescriptor(),
            // SphereColliderComponent::CreateDescriptor(),
            // CapsuleColliderComponent::CreateDescriptor(),
            ShapeColliderComponent::CreateDescriptor(),
            RigidBodyComponent::CreateDescriptor(),
            StaticRigidBodyComponent::CreateDescriptor(),
        };

        return descriptors;
    }
} // namespace JoltPhysics
