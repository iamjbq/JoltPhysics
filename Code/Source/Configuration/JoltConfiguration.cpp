
#include <JoltPhysics/Configuration/JoltConfiguration.h>

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace JoltPhysics
{
    namespace JoltInternal
    {
        // To work with Jolt 32-bit ObjectLayers, will
        AzPhysics::CollisionConfiguration CreateDefaultCollisionConfiguration()
        {
            AzPhysics::CollisionConfiguration configuration;
            configuration.m_collisionLayers.SetName(AzPhysics::CollisionLayer::Default, "Default");

            configuration.m_collisionGroups.CreateGroup("All", AzPhysics::CollisionGroup::All, AzPhysics::CollisionGroups::Id(), true);
            configuration.m_collisionGroups.CreateGroup("None", AzPhysics::CollisionGroup::None, AzPhysics::CollisionGroups::Id::Create(), true);

            return configuration;
        }
    }

    AZ_CLASS_ALLOCATOR_IMPL(JoltSystemConfiguration, AZ::SystemAllocator, 0)

    /*static*/ void JoltSystemConfiguration::Reflect(AZ::ReflectContext* context)
    {
        AzPhysics::SystemConfiguration::Reflect(context);

        if (auto* serializeContext = azdynamic_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<JoltPhysics::JoltSystemConfiguration, AzPhysics::SystemConfiguration>()
                ->Version(1)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                // this is needed so the edit context of AzPhysics::SystemConfiguration can be used.
                editContext->Class<JoltPhysics::JoltSystemConfiguration>("System Configuration", "Jolt system configuration")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    JoltSystemConfiguration JoltSystemConfiguration::CreateDefault()
    {
        JoltSystemConfiguration systemConfig;
        systemConfig.m_collisionConfig = JoltInternal::CreateDefaultCollisionConfiguration();
        return systemConfig;
    };

    bool JoltSystemConfiguration::operator==(const JoltSystemConfiguration& other) const
    {
        return AzPhysics::SystemConfiguration::operator==(other);
    }

    bool JoltSystemConfiguration::operator!=(const JoltSystemConfiguration& other) const
    {
        return !(*this == other);
    }
}
