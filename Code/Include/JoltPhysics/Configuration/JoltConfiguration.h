#pragma once
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzFramework/Physics/Configuration/SystemConfiguration.h>

namespace AZ
{
    class ReflectContext;
}

namespace JoltPhysics
{
    struct JoltSystemConfiguration : public AzPhysics::SystemConfiguration
    {
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_RTTI(JoltPhysics::JoltSystemConfiguration, "{C7B81915-F277-4286-8D26-46739260423D}")
        static void Reflect(AZ::ReflectContext* context);

        static JoltSystemConfiguration CreateDefault();

        bool operator==(const JoltSystemConfiguration& other) const;
        bool operator!=(const JoltSystemConfiguration& other) const;
    };
}
