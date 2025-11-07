#pragma once

#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>

namespace JoltPhysics
{
    //! Class for in-editor Jolt Static Rigid Body Component.
    class EditorStaticRigidBodyComponent : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(
            EditorStaticRigidBodyComponent, "{EC28A5BF-890E-48DC-879B-84C1B2ADBC95}", AzToolsFramework::Components::EditorComponentBase);
        static void Reflect(AZ::ReflectContext* context);

        EditorStaticRigidBodyComponent() = default;
        ~EditorStaticRigidBodyComponent() = default;

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // EditorComponentBase
        void BuildGameEntity(AZ::Entity* gameEntity) override;
    };
} // namespace JoltPhysics
