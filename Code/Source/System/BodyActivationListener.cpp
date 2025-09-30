
#include <System/BodyActivationListener.h>

#include <AzCore/Debug/Trace.h>

namespace JoltPhysics
{
    void JoltBodyActivationListener::OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData)
    {
        AZ_Info("JoltBodyActivationListener", "A body got activated")
    }

    void JoltBodyActivationListener::OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData)
    {
        AZ_Info("JoltBodyActivationListener", "A body went to sleep")
    }
}
