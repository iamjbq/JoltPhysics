
#include "JoltSystemInterface.h"

namespace JoltPhysics
{
    SystemHandle JoltSystemInterface::GetSystemHandle([[maybe_unused]] const AZStd::string& SystemName)
    {
        // TODO: Need to add this function in WorldSimulationOwner
        SystemHandle result = InvalidSystemHandle;
        return result;
    }

    System* JoltSystemInterface::GetSystem([[maybe_unused]] SystemHandle handle)
    {
        // TODO: Need to add this function in WorldSimulationOwner
        return nullptr;
    }

    void JoltSystemInterface::StartUpdate([[maybe_unused]] SystemHandle SystemHandle, [[maybe_unused]] float deltatime)
    {
        
    }

    void JoltSystemInterface::FinishUpdate([[maybe_unused]] SystemHandle SystemHandle)
    {

    }
}
