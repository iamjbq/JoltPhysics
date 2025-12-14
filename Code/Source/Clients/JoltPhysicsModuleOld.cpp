
#include <JoltPhysics/JoltPhysicsTypeIds.h>
#include <JoltPhysicsModule.h>
#include "JoltPhysicsSystemComponent.h"

namespace JoltPhysics
{
    class JoltPhysicsModuleOld
        : public JoltPhysicsModuleInterface
    {
    public:
        AZ_RTTI(JoltPhysicsModuleOld, JoltPhysicsModuleTypeId, JoltPhysicsModuleInterface);
        AZ_CLASS_ALLOCATOR(JoltPhysicsModuleOld, AZ::SystemAllocator);
    };
}// namespace JoltPhysics

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), JoltPhysics::JoltPhysicsModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_JoltPhysics, JoltPhysics::JoltPhysicsModule)
#endif
