#pragma once

#include <AzCore/EBus/EBus.h>

namespace JoltPhysics
{
    namespace Editor
    {
        /// Request bus for communicating with the configuration window.
        /// Ensure the configuration window has been opened by 
        /// calling EditorRequests::OpenViewPane before using this bus.
        class ConfigurationWindowRequests
            : public AZ::EBusTraits
        {
        public:
            
            /// Shows the collision layers tab in the configuration window
            virtual void ShowCollisionLayersTab() = 0;

            /// Shows the collision groups tab in the configuration window
            virtual void ShowCollisionGroupsTab() = 0;

            /// Shows the global settings tab in the configuration window
            virtual void ShowGlobalSettingsTab() = 0;

        };

        using ConfigurationWindowRequestBus = AZ::EBus<ConfigurationWindowRequests>;
    }
}
