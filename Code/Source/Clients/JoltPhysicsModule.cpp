
#include <JoltPhysics/JoltPhysicsTypeIds.h>
#include <JoltPhysicsModuleInterface.h>
#include "JoltPhysicsSystemComponent.h"

namespace JoltPhysics
{
    class JoltPhysicsModule
        : public JoltPhysicsModuleInterface
    {
    public:
        AZ_RTTI(JoltPhysicsModule, JoltPhysicsModuleTypeId, JoltPhysicsModuleInterface);
        AZ_CLASS_ALLOCATOR(JoltPhysicsModule, AZ::SystemAllocator);
    };
}// namespace JoltPhysics

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), JoltPhysics::JoltPhysicsModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_JoltPhysics, JoltPhysics::JoltPhysicsModule)
#endif
