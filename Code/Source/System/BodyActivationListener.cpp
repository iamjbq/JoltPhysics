
#include <System/BodyActivationListener.h>

#include <AzCore/Debug/Trace.h>

namespace JoltPhysics
{
    void JoltBodyActivationListener::OnBodyActivated([[maybe_unused]] const JPH::BodyID& inBodyID, [[maybe_unused]] JPH::uint64 inBodyUserData)
    {
        AZ_Info("JoltBodyActivationListener", "A body got activated")
    }

    void JoltBodyActivationListener::OnBodyDeactivated([[maybe_unused]] const JPH::BodyID& inBodyID, [[maybe_unused]] JPH::uint64 inBodyUserData)
    {
        AZ_Info("JoltBodyActivationListener", "A body went to sleep")
    }
}
