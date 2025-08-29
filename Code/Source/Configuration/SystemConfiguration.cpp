
#include "SystemConfiguration.h"

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(SystemConfiguration, AZ::SystemAllocator);

    void SystemConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<SystemConfiguration>()
                ->Version(2)
                ->Field("Name", &SystemConfiguration::m_systemName)
                ->Field("WorldBounds", &SystemConfiguration::m_worldBounds)
                ->Field("Gravity", &SystemConfiguration::m_gravity)
                // ->Field("EnableCcd", &SystemConfiguration::m_enableCcd)
                // ->Field("MaxCcdPasses", &SystemConfiguration::m_maxCcdPasses)
                // ->Field("EnableCcdResweep", &SystemConfiguration::m_enableCcdResweep)
                // ->Field("EnableActiveActors", &SystemConfiguration::m_enableActiveActors)
                // ->Field("EnablePcm", &SystemConfiguration::m_enablePcm)
                // ->Field("BounceThresholdVelocity", &SystemConfiguration::m_bounceThresholdVelocity)
                ;

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<SystemConfiguration>("Scene Configuration", "Default scene configuration")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemConfiguration::m_worldBounds, "World Bounds", "World bounds")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemConfiguration::m_gravity, "Gravity", "Gravity")
                    // ->ClassElement(AZ::Edit::ClassElements::Group, "Continuous Collision Detection")
                    // ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    // ->DataElement(AZ::Edit::UIHandlers::Default, &SystemConfiguration::m_enableCcd, "Enable CCD", "Enabled continuous collision detection in the world")
                    // ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)
                    // ->DataElement(AZ::Edit::UIHandlers::Default, &SystemConfiguration::m_maxCcdPasses,
                    //     "Max CCD Passes", "Maximum number of continuous collision detection passes")
                    // ->Attribute(AZ::Edit::Attributes::Visibility, &SystemConfiguration::GetCcdVisibility)
                    // ->Attribute(AZ::Edit::Attributes::Min, 1u)
                    // ->DataElement(AZ::Edit::UIHandlers::Default, &SystemConfiguration::m_enableCcdResweep,
                    //     "Enable CCD Resweep", "Enable a more accurate but more expensive continuous collision detection method")
                    // ->Attribute(AZ::Edit::Attributes::Visibility, &SystemConfiguration::GetCcdVisibility)
                    // ->EndGroup()
                    //
                    // ->DataElement(AZ::Edit::UIHandlers::Default, &SystemConfiguration::m_enablePcm, "Persistent Contact Manifold", "Enabled the persistent contact manifold narrow-phase algorithm")
                    // ->DataElement(AZ::Edit::UIHandlers::Default, &SystemConfiguration::m_bounceThresholdVelocity,
                    //     "Bounce Threshold Velocity", "Relative velocity below which colliding objects will not bounce")
                    // ->Attribute(AZ::Edit::Attributes::Min, 0.01f)
                    ;
            }
        }
    }

    SystemConfiguration SystemConfiguration::CreateDefault()
    {
        return SystemConfiguration();
    }

    AZ::u32 SystemConfiguration::OnMaxTimeStepChanged()
    {
        m_fixedTimestep = AZStd::GetMin(m_fixedTimestep, GetFixedTimeStepMax()); //since m_maxTimeStep has changed, m_fixedTimeStep might be larger then the max.
        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }

    float SystemConfiguration::GetFixedTimeStepMax() const
    {
        return m_maxTimestep;
    }
}
