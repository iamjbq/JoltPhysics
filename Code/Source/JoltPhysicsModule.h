#include <JoltPhysics/JoltPhysicsTypeIds.h>

#include <AzCore/Memory/Memory.h>
#include <AzCore/Memory/Memory_fwd.h>
#include <AzCore/Module/Module.h>
#include <AzCore/RTTI/RTTIMacros.h>
#include <AzCore/RTTI/TypeInfoSimple.h>

#include <System/JoltSystem.h>

namespace AZ
{
    class DynamicModuleHandle;
}

namespace JoltPhysics
{
    class JoltPhysicsModule
        : public AZ::Module
    {
    public:
        // AZ_TYPE_INFO_WITH_NAME_DECL(JoltPhysicsModuleInterface)
        // AZ_RTTI_NO_TYPE_INFO_DECL()
        // AZ_CLASS_ALLOCATOR_DECL
        AZ_RTTI(JoltPhysicsModule, JoltPhysicsModuleTypeId, AZ::Module);
        AZ_CLASS_ALLOCATOR(JoltPhysicsModule, AZ::SystemAllocator);

        JoltPhysicsModule();
        virtual ~JoltPhysicsModule();

        AZ::ComponentTypeList GetRequiredSystemComponents() const override;

    private:
        void LoadModules();
        void UnloadModules();

        /// Required modules to load/unload when Jolt Gem module is created/destroyed
        AZStd::vector<AZStd::unique_ptr<AZ::DynamicModuleHandle>> m_modules;
        JoltSystem m_joltSystem;
    };
}// namespace JoltPhysics
