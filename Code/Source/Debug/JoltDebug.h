

#pragma once

#include <AzCore/Interface/Interface.h>
#include <JoltPhysics/Debug/JoltDebugInterface.h>

namespace JoltPhysics
{
    namespace Debug
    {
        // TODO: This class may not be correct as we have JoltDebugRenderer to inherit from, but tbd
        class JoltDebug
            : AZ::Interface<JoltDebugInterface>::Registrar
        {
        public:
            JoltDebug() = default;

            // JoltDebugInterface
            void Initialize(const DebugConfiguration& config) override;
            void UpdateDebugConfiguration(const DebugConfiguration& config) override;
            const DebugConfiguration& GetDebugConfiguration() const override;
            const DebugDisplayData& GetDebugDisplayData() const override;
            void UpdateColliderProximityVisualization(const ColliderProximityVisualization& data) override;

        private:
            DebugConfiguration m_config;
        };
    }
} // JoltPhysics
