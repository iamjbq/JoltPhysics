#include <JoltPhysics/Configuration/JoltConfiguration.h>

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace JoltPhysics
{
    namespace JoltInternal
    {
        AzPhysics::CollisionConfiguration CreateDefaultCollisionConfiguration()
        {
            AzPhysics::CollisionConfiguration configuration;
            configuration.m_collisionLayers.SetName(AzPhysics::CollisionLayer::Default, "Default");

            configuration.m_collisionGroups.CreateGroup("All", AzPhysics::CollisionGroup::All, AzPhysics::CollisionGroups::Id(), true);
            configuration.m_collisionGroups.CreateGroup("None", AzPhysics::CollisionGroup::None, AzPhysics::CollisionGroups::Id::Create(), true);

            return configuration;
        }
    }

    void SystemInitSettings::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azdynamic_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<SystemInitSettings>()
                ->Version(1)->Version(1)
                ->Field("MaxBodies", &SystemInitSettings::m_maxBodies)
                ->Field("NumBodyMutexes", &SystemInitSettings::m_numBodyMutexes)
                ->Field("MaxBodyPairs", &SystemInitSettings::m_maxBodyPairs)
                ->Field("MaxContactConstraints", &SystemInitSettings::m_maxContactConstraints)
                ->Field("CollisionSteps", &SystemInitSettings::m_collisionSteps)
                ->Field("AllocationArenaSize", &SystemInitSettings::m_allocationArenaSize)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<SystemInitSettings>("Jolt System Init Settings", "Initialization and system update settings.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_maxBodies,
                        "MaxBodies", "Maximum bodies per physics scene.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_numBodyMutexes,
                        "NumBodyMutexes", "Number of mutexes to protect bodies from concurrent access.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_maxBodyPairs,
                        "MaxBodyPairs", "Number of body pairs that can be queued at any time.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_maxContactConstraints,
                        "MaxContactConstraints", "Number of collisions between bodies that can be handled each update.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_collisionSteps,
                        "CollisionSteps", "Number of collision steps to perform each update.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_allocationArenaSize,
                        "AllocationArenaSize", "Maximum memory to allocate per physics update.")
                    ;
            }
        }
    }

    bool SystemInitSettings::operator==(const SystemInitSettings& other) const
    {
        return m_maxBodies == other.m_maxBodies
            && m_numBodyMutexes == other.m_numBodyMutexes
            && m_maxContactConstraints == other.m_maxContactConstraints
            && m_maxBodyPairs == other.m_maxBodyPairs
            && m_collisionSteps == other.m_collisionSteps
            && m_allocationArenaSize == other.m_allocationArenaSize;
    }

    bool SystemInitSettings::operator!=(const SystemInitSettings& other) const
    {
        return !(*this == other);
    }

    AZ_CLASS_ALLOCATOR_IMPL(SystemInitSettings, AZ::SystemAllocator)
    AZ_CLASS_ALLOCATOR_IMPL(JoltSystemConfiguration, AZ::SystemAllocator)

    /*static*/ void JoltSystemConfiguration::Reflect(AZ::ReflectContext* context)
    {
        AzPhysics::SystemConfiguration::Reflect(context);
        SystemInitSettings::Reflect(context);

        if (auto* serializeContext = azdynamic_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<JoltSystemConfiguration, AzPhysics::SystemConfiguration>()
                ->Version(2)
                ->Field("SystemInitSettings", &JoltSystemConfiguration::m_systemInitSettings)
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
        return AzPhysics::SystemConfiguration::operator==(other)
            && m_systemInitSettings == other.m_systemInitSettings
            ;
    }

    bool JoltSystemConfiguration::operator!=(const JoltSystemConfiguration& other) const
    {
        return !(*this == other);
    }
}
