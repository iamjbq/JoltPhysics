
#include <JoltPhysics/JoltPhysicsTypeIds.h>
#include <JoltPhysicsModuleInterface.h>
#include "JoltPhysicsEditorSystemComponent.h"
#include "Clients/EditorRigidBodyComponent.h"

namespace JoltPhysics
{
    class JoltPhysicsEditorModule
        : public JoltPhysicsModuleInterface
    {
    public:
        AZ_RTTI(JoltPhysicsEditorModule, JoltPhysicsEditorModuleTypeId, JoltPhysicsModuleInterface);
        AZ_CLASS_ALLOCATOR(JoltPhysicsEditorModule, AZ::SystemAllocator);

        JoltPhysicsEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                JoltPhysicsEditorSystemComponent::CreateDescriptor(),
                EditorRigidBodyComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<JoltPhysicsEditorSystemComponent>(),
            };
        }
    };
}// namespace JoltPhysics

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), JoltPhysics::JoltPhysicsEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_JoltPhysics_Editor, JoltPhysics::JoltPhysicsEditorModule)
#endif
