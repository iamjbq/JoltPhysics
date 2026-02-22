
#include <AzCore/std/functional.h>
#include <Debug/JoltDebug.h>

#include <AzFramework/API/ApplicationAPI.h>
#include <AzFramework/StringFunc/StringFunc.h>
#include <AzCore/std/string/conversions.h>
#include <AzCore/Utils/Utils.h>
#include <AzCore/std/time.h>

namespace JoltPhysics
{
    namespace Debug
    {
        void JoltDebug::Initialize(const DebugConfiguration& config)
        {
            m_config = config;
        }

        void JoltDebug::UpdateDebugConfiguration(const DebugConfiguration& config)
        {
            const bool signalDebugDataChanged = m_config.m_debugDisplayData != config.m_debugDisplayData;
            // const bool signalPvdConfigChanged = m_config.m_pvdConfigurationData != config.m_pvdConfigurationData;
            if (signalDebugDataChanged
                // || signalPvdConfigChanged
                )
            {
                m_config = config;
                if (signalDebugDataChanged)
                {
                    m_debugDisplayDataChangedEvent.Signal(config.m_debugDisplayData);
                }
                // if (signalPvdConfigChanged)
                // {
                //     m_pvdConfigurationChangedEvent.Signal(config.m_pvdConfigurationData);
                // }
            }
        }

        const DebugConfiguration& JoltDebug::GetDebugConfiguration() const
        {
            return m_config;
        }

        const DebugDisplayData& JoltDebug::GetDebugDisplayData() const
        {
            return m_config.m_debugDisplayData;
        }

        void JoltDebug::UpdateColliderProximityVisualization(const ColliderProximityVisualization& data)
        {
            if (m_config.m_debugDisplayData.m_colliderProximityVisualization != data)
            {
                m_config.m_debugDisplayData.m_colliderProximityVisualization = data;
                m_colliderProximityVisualizationChangedEvent.Signal(data);
            }
        }
    }
} // JoltPhysics
