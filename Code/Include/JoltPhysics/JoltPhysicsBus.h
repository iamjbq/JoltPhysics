
#pragma once

#include <JoltPhysics/JoltPhysicsTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace JoltPhysics
{
    class JoltPhysicsRequests
    {
    public:
        AZ_RTTI(JoltPhysicsRequests, JoltPhysicsRequestsTypeId);
        virtual ~JoltPhysicsRequests() = default;
        // Put your public methods here
    };

    class JoltPhysicsBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using JoltPhysicsRequestBus = AZ::EBus<JoltPhysicsRequests, JoltPhysicsBusTraits>;
    using JoltPhysicsInterface = AZ::Interface<JoltPhysicsRequests>;

} // namespace JoltPhysics
