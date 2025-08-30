
#include "JoltPhysicsSystemInterface.h"

namespace JoltPhysics
{
    SystemHandle JoltPhysicsSystemInterface::GetSystemHandle([[maybe_unused]] const AZStd::string& SystemName)
    {
        SystemHandle result = InvalidSystemHandle;
        return result;
    }

    System* JoltPhysicsSystemInterface::GetSystem([[maybe_unused]] SystemHandle handle)
    {
        return nullptr;
    }

    void JoltPhysicsSystemInterface::StartUpdate([[maybe_unused]] SystemHandle SystemHandle, [[maybe_unused]] float deltatime)
    {

    }

    void JoltPhysicsSystemInterface::FinishUpdate([[maybe_unused]] SystemHandle SystemHandle)
    {

    }
}
