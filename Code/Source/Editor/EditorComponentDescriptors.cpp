#include <Editor/EditorComponentDescriptors.h>

#include <Clients/JoltPhysicsEditorSystemComponent.h>
#include <Clients/EditorPrimitiveShapeColliderComponent.h>
#include <Clients/EditorMeshColliderComponent.h>
#include <Clients/EditorRigidBodyComponent.h>
#include <Clients/EditorStaticRigidBodyComponent.h>
#include <Pipeline/MeshBehavior.h>
#include <Pipeline/MeshExporter.h>
#include <Editor/JoltEditorSettingsRegistryManager.h>

namespace JoltPhysics
{
    AZStd::list<AZ::ComponentDescriptor*> GetEditorDescriptors()
    {
        AZStd::list<AZ::ComponentDescriptor*> descriptors =
        {
            JoltPhysicsEditorSystemComponent::CreateDescriptor(),
            JoltPhysicsSystemComponent::CreateDescriptor(),
            EditorPrimitiveShapeColliderComponent::CreateDescriptor(),
            EditorMeshColliderComponent::CreateDescriptor(),
            EditorRigidBodyComponent::CreateDescriptor(),
            EditorStaticRigidBodyComponent::CreateDescriptor(),
            Pipeline::MeshBehavior::CreateDescriptor(),
            Pipeline::MeshExporter::CreateDescriptor()
        };

        return descriptors;
    }

} // namespace JoltPhysics
