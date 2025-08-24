
#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace JoltPhysics
{
    class JoltEditorRigidBodyRequests
        : public AZ::ComponentBus
    {
    public:
        AZ_RTTI(JoltPhysics::JoltEditorRigidBodyRequests, "{FF182CCC-EDA8-4579-81A3-5F6DBF903187}");

        // Put your public request methods here.
        
        // Put notification events here. Examples:
        // void RegisterEvent(AZ::EventHandler<...> notifyHandler);
        // AZ::Event<...> m_notifyEvent1;
        
    };

    using JoltEditorRigidBodyRequestBus = AZ::EBus<JoltEditorRigidBodyRequests>;

} // namespace JoltPhysics
