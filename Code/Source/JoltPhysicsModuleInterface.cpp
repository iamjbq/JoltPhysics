
#include <JoltPhysicsModuleInterface.h>
#include <AzCore/Memory/Memory.h>

#include <JoltPhysics/JoltPhysicsTypeIds.h>
#include <Clients/JoltPhysicsSystemComponent.h>
#include <Clients/BaseColliderComponent.h>
#include <Clients/RigidBodyComponent.h>
#include <Clients/StaticRigidBodyComponent.h>

namespace JoltPhysics
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(JoltPhysicsModuleInterface,
        "JoltPhysicsModuleInterface", JoltPhysicsModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(JoltPhysicsModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(JoltPhysicsModuleInterface, AZ::SystemAllocator);

    JoltPhysicsModuleInterface::JoltPhysicsModuleInterface()
    {
        AZ_TracePrintf("PhysicsModuleInterface", "Constructed\n")
        
        // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
        // Add ALL components descriptors associated with this gem to m_descriptors.
        // This will associate the AzTypeInfo information for the components with the SerializeContext, BehaviorContext and EditContext.
        // This happens through the [MyComponent]::Reflect() function.
        m_descriptors.insert(m_descriptors.end(), {
            JoltPhysicsSystemComponent::CreateDescriptor(),
            BaseColliderComponent::CreateDescriptor(),
            RigidBodyComponent::CreateDescriptor(),
            StaticRigidBodyComponent::CreateDescriptor(),
            });
    }

    JoltPhysicsModuleInterface::~JoltPhysicsModuleInterface()
    {
        m_joltSystem.Shutdown();
    }

    AZ::ComponentTypeList JoltPhysicsModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<JoltPhysicsSystemComponent>(),
        };
    }
} // namespace JoltPhysics
