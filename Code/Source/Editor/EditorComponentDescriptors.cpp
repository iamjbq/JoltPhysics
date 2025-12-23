#include <Editor/EditorComponentDescriptors.h>

#include <Clients/JoltPhysicsEditorSystemComponent.h>
#include <Clients/EditorShapeColliderComponent.h>
#include <Clients/EditorRigidBodyComponent.h>
#include <Clients/EditorStaticRigidBodyComponent.h>
#include <Editor/JoltEditorSettingsRegistryManager.h>

namespace JoltPhysics
{
    AZStd::list<AZ::ComponentDescriptor*> GetEditorDescriptors()
    {
        AZStd::list<AZ::ComponentDescriptor*> descriptors =
        {
            JoltPhysicsEditorSystemComponent::CreateDescriptor(),
            JoltPhysicsSystemComponent::CreateDescriptor(),
            EditorShapeColliderComponent::CreateDescriptor(),
            EditorRigidBodyComponent::CreateDescriptor(),
            EditorStaticRigidBodyComponent::CreateDescriptor(),
        };

        return descriptors;
    }

} // namespace JoltPhysics
