
#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace JoltPhysics
{
    class JoltRigidBodyRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(JoltPhysics::JoltRigidBodyRequests, "{870209D5-59AA-48E5-887C-E2B0AF2149D0}");

        // Put your public request methods here.
        
        // Put notification events here. Examples:
        // void RegisterEvent(AZ::EventHandler<...> notifyHandler);
        // AZ::Event<...> m_notifyEvent1;
        
    };

    using JoltRigidBodyRequestBus = AZ::EBus<JoltRigidBodyRequests>;

} // namespace JoltPhysics
