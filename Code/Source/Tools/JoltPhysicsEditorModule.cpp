
#include <JoltPhysics/JoltPhysicsTypeIds.h>
#include <JoltPhysicsModuleInterface.h>
#include <Clients/JoltPhysicsSystemComponent.h>
#include <Tools/JoltPhysicsEditorSystemComponent.h>
#include <Clients/EditorShapeColliderComponent.h>
#include <Clients/EditorRigidBodyComponent.h>
#include <Clients/EditorStaticRigidBodyComponent.h>
#include <Editor/JoltEditorSettingsRegistryManager.h>

namespace JoltPhysics
{
    class JoltSystem;
    
    class JoltPhysicsEditorModule
        : public JoltPhysicsModuleInterface
    {
    public:
        AZ_RTTI(JoltPhysicsEditorModule, JoltPhysicsEditorModuleTypeId, JoltPhysicsModuleInterface);
        AZ_CLASS_ALLOCATOR(JoltPhysicsEditorModule, AZ::SystemAllocator);

        JoltPhysicsEditorModule()
            : m_joltSystem(AZStd::make_unique<JoltEditorSettingsRegistryManager>())
        {
            static_assert(alignof(JoltPhysics::JoltSystemConfiguration) == 16);
            static_assert(alignof(JoltPhysics::JoltSystem) == 16);
            
            AZ_TracePrintf("JoltPhysicsEditorModule", "Constructed\n")
            
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                JoltPhysicsSystemComponent::CreateDescriptor(),
                JoltPhysicsEditorSystemComponent::CreateDescriptor(),
                EditorShapeColliderComponent::CreateDescriptor(),
                EditorRigidBodyComponent::CreateDescriptor(),
                EditorStaticRigidBodyComponent::CreateDescriptor(),
            });
        }

        ~JoltPhysicsEditorModule()
        {
            m_joltSystem.Shutdown();
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<JoltPhysicsSystemComponent>(),
                azrtti_typeid<JoltPhysicsEditorSystemComponent>(),
            };
        }

    private:
        JoltSystem m_joltSystem;
    };
}// namespace JoltPhysics

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME, _Editor), JoltPhysics::JoltPhysicsEditorModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_JoltPhysics_Editor, JoltPhysics::JoltPhysicsEditorModule)
#endif
