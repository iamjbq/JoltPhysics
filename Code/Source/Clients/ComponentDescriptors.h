
#pragma once

#include <AzCore/std/containers/list.h>

namespace AZ
{
    class ComponentDescriptor;
}

namespace JoltPhysics
{
    AZStd::list<AZ::ComponentDescriptor*> GetDescriptors();
} // namespace JoltPhysics
