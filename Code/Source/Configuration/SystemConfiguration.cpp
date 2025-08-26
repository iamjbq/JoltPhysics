
#include "SystemConfiguration.h"

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(SystemConfiguration, AZ::SystemAllocator);

    SystemConfiguration SystemConfiguration::CreateDefault()
    {
        return SystemConfiguration();
    }
}
