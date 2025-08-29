
#include "SystemInterface.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Interface/Interface.h>

#include "Configuration/SystemConfiguration.h"

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(System, AZ::SystemAllocator)

    void System::Reflect(AZ::ReflectContext* context)
    {
        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<System>("PhysicsSystem")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "physics")
                ->Attribute(AZ::Script::Attributes::Category, "Physics")
                // ->Method("GetOnGravityChangeEvent", getOnGravityChange)
                //     ->Attribute(AZ::Script::Attributes::AzEventDescription, gravityChangedEventDescription)
                // ->Method("QuerySystem", [](System* self, const SystemQueryRequest* request)
                // {
                //     return self->QuerySystem(request);
                // })
                ;
        }
    }

    System::System(const SystemConfiguration& config)
        : m_id(config.m_systemName)
    {

    }

    const AZ::Crc32& System::GetId() const
    {
        return m_id;
    }
}
