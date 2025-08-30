
#pragma once

#include <AzCore/Interface/Interface.h>

#include <System/SystemInterface.h>

namespace JoltPhysics
{
    class JoltSystemInterface
        : public AZ::Interface<JoltPhysics::SystemInterface>::Registrar
    {
        public:
            explicit JoltSystemInterface();

            // SystemInterface interface ...
            SystemHandle GetSystemHandle(const AZStd::string& SystemName) override;
            System* GetSystem(SystemHandle handle) override;
            void StartUpdate(SystemHandle SystemHandle, float deltatime) override;
            void FinishUpdate(SystemHandle SystemHandle) override;
    };
}
