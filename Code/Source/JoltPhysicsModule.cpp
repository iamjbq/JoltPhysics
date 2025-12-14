
#include <JoltPhysicsModule.h>
#include <AzCore/Module/DynamicModuleHandle.h>

#include <Clients/JoltPhysicsSystemComponent.h>
#include <Clients/ComponentDescriptors.h>
#include <Configuration/JoltSettingsRegistryManager.h>

#if defined(JOLT_EDITOR)
#include <Tools/JoltPhysicsEditorSystemComponent.h>
#include <Editor/EditorComponentDescriptors.h>
#include <Editor/JoltEditorSettingsRegistryManager.h>
#endif

namespace JoltPhysics
{
    // AZ_TYPE_INFO_WITH_NAME_IMPL(JoltPhysicsModuleInterface,
    //     "JoltPhysicsModuleInterface", JoltPhysicsModuleInterfaceTypeId);
    // AZ_RTTI_NO_TYPE_INFO_IMPL(JoltPhysicsModuleInterface, AZ::Module);
    // AZ_CLASS_ALLOCATOR_IMPL(JoltPhysicsModuleInterface, AZ::SystemAllocator);

    JoltPhysicsModule::JoltPhysicsModule()
        : AZ::Module()
#if defined(JOLT_EDITOR)
        , m_joltSystem(AZStd::make_unique<JoltEditorSettingsRegistryManager>())
#else
        , m_joltSystem(AZStd::make_unique<JoltSettingsRegistryManager>())
#endif
    {
        static_assert(alignof(JoltPhysics::JoltSystemConfiguration) == 16);
        static_assert(alignof(JoltPhysics::JoltSystem) == 16);

        LoadModules();

        // This will associate the AzTypeInfo information for the components with the SerializeContext, BehaviorContext and EditContext.
        AZStd::list<AZ::ComponentDescriptor*> descriptorsToAdd = GetDescriptors();
        m_descriptors.insert(m_descriptors.end(), descriptorsToAdd.begin(), descriptorsToAdd.end());
#if defined(JOLT_EDITOR)
        AZStd::list<AZ::ComponentDescriptor*> editorDescriptorsToAdd = GetEditorDescriptors();
        m_descriptors.insert(m_descriptors.end(), editorDescriptorsToAdd.begin(), editorDescriptorsToAdd.end());
#endif
    }

    JoltPhysicsModule::~JoltPhysicsModule()
    {
        m_joltSystem.Shutdown();

        AZ::GetGlobalSerializeContextModule().Cleanup();

        UnloadModules();
    }

    AZ::ComponentTypeList JoltPhysicsModule::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<JoltPhysicsSystemComponent>(),
#if defined(JOLT_EDITOR)
            azrtti_typeid<JoltPhysicsEditorSystemComponent>(),
#endif
        };
    }

    void JoltPhysicsModule::LoadModules()
    {
#if defined(JOLT_EDITOR)
        {
            AZStd::unique_ptr<AZ::DynamicModuleHandle> sceneCoreModule = AZ::DynamicModuleHandle::Create("SceneCore");
            [[maybe_unused]] bool ok = sceneCoreModule->Load(AZ::DynamicModuleHandle::LoadFlags::InitFuncRequired);
            AZ_Error("JoltPhysics::JoltPhysicsModuleInterface", ok, "Error loading SceneCore module");

            m_modules.push_back(AZStd::move(sceneCoreModule));
        }
#endif
    }

    void JoltPhysicsModule::UnloadModules()
    {
        // Unload modules in reserve order that were loaded
        for (auto module = m_modules.rbegin(); module != m_modules.rend(); ++module)
        {
            module->reset();
        }
        m_modules.clear();
    }
} // namespace JoltPhysics

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), JoltPhysics::JoltPhysicsModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_JoltPhysics, JoltPhysics::JoltPhysicsModule)
#endif
