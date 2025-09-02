#pragma once

#include <AzCore/EBus/Event.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/RTTI/ReflectContext.h>

#include <JoltPhysics/JoltPhysicsTypes.h>
#include <System/JoltSystemOld.h>
#include <Configuration/SystemConfiguration.h>


namespace JoltPhysics
{
    class WorldSimulationInterface
    {
        public:
            AZ_RTTI(WorldSimulationInterface, "{D7974928-FE2B-44E3-BAA3-86B336A41DC9}");

            WorldSimulationInterface() = default;
            virtual ~WorldSimulationInterface() = default;
            AZ_DISABLE_COPY_MOVE(WorldSimulationInterface);

            static void Reflect(AZ::ReflectContext* context);

            virtual void Initialize() = 0;
            virtual void Update(float inDeltaTime) = 0;
            virtual SystemHandle AddPhysicsSystem(const SystemConfiguration& config) = 0;
    };
}

