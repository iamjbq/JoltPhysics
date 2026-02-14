#include <JoltPhysics/Configuration/JoltConfiguration.h>

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Physics/NameConstants.h>

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
                ->Version(1)
                ->Field("VelocitySolverSteps", &SystemInitSettings::m_numVelocitySteps)
                ->Field("PositionSolverSteps", &SystemInitSettings::m_numPositionSteps)
                ->Field("AllowSleeping", &SystemInitSettings::m_allowSleeping)
                ->Field("TimeSleepThreshold", &SystemInitSettings::m_timeBeforeSleep)
                ->Field("VelocitySleepThreshold", &SystemInitSettings::m_pointVelocitySleepThreshold)
                ->Field("DeterministicSimulation", &SystemInitSettings::m_deterministicSimulation)
                ->Field("CollisionSteps", &SystemInitSettings::m_collisionSteps)
                ->Field("SpeculativeContactDistance", &SystemInitSettings::m_speculativeContactDistance)
                ->Field("PenetrationSlop", &SystemInitSettings::m_penetrationSlop)
                ->Field("BaumgarteStabilizationFactor", &SystemInitSettings::m_baumgarte)
                ->Field("UseBodyPairContactCache", &SystemInitSettings::m_useBodyPairContactCache)
                ->Field("BodyPairCacheDistanceThreshold", &SystemInitSettings::m_bodyPairCacheMaxDeltaPosition)
                ->Field("BodyPairCacheRotationThreshold", &SystemInitSettings::m_bodyPairCacheCosMaxDeltaRotation)
                ->Field("MaxBodies", &SystemInitSettings::m_maxBodies)
                ->Field("NumBodyMutexes", &SystemInitSettings::m_numBodyMutexes)
                ->Field("MaxBodyPairs", &SystemInitSettings::m_maxBodyPairs)
                ->Field("MaxContactConstraints", &SystemInitSettings::m_maxContactConstraints)
                ->Field("AllocationArenaSize", &SystemInitSettings::m_allocationArenaSize)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<SystemInitSettings>("Jolt System Init Settings", "Initialization and system update settings.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Simulation")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_numVelocitySteps,
                    "Velocity Solver Steps", "Number of solver velocity iterations to run.\n"
                        "Note that this needs to be >= 2 in order for friction to work")
                        ->Attribute(AZ::Edit::Attributes::Min, 2)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_numPositionSteps,
                        "Velocity Solver Steps", "Number of solver position iterations to run.")
                        ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_allowSleeping,
                        "Allow Sleeping", "Globally enable whether bodies can ever sleep.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_timeBeforeSleep,
                        "Time Sleep Threshold", "Time before object is allowed to go to sleep.")
                        ->Attribute(AZ::Edit::Attributes::Suffix, " s")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_pointVelocitySleepThreshold,
                        "Velocity Sleep Threshold", "The velocity before an object is allowed to go to sleep.\n"
                        "Tracks body center of mass and center point of the body bounding box faces which are furthest away from center.\n"
                        "All three points must be above the velocity threshold.")
                        ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetSpeedUnit())
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_deterministicSimulation,
                        "Deterministic Simulation", "Whether the simulation should run deterministically.\n"
                        "Default behaviour is deterministic, but can be disabled for increased performance at the cost of variation")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_collisionSteps,
                        "Collision Steps", "The number of collision steps for the solver to perform per update call.")
                        ->Attribute(AZ::Edit::Attributes::Min, 1)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_speculativeContactDistance,
                        "Speculative Contact Distance", "Radius around objects inside which speculative contact points will be detected.\n"
                        "Note: If too big, you will get ghost collisions as speculative contacts are based on the closest points during the collision detection.")
                        ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetLengthUnit())
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_penetrationSlop,
                        "Penetration Slop", "How far bodies are allowed to sink into each other.")
                        ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetLengthUnit())
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_baumgarte,
                        "Baumgarte Stabilization Factor", "How much of the position error to 'fix' in 1 update.\n"
                        "Note: 0 = nothing, 1 = 100%")
                        ->Attribute(AZ::Edit::Attributes::Min, 0)
                        ->Attribute(AZ::Edit::Attributes::Max, 1)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_useBodyPairContactCache,
                        "Use Body Pair Contact Cache", "Removes the need for narrow phase collision detection when orientation between two bodies didn't change.\n"
                        "Note: This is also called Persistent Contact Manifold (PCM), and can be enabled in Scene Settings, but offers additional controls below.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_bodyPairCacheMaxDeltaPosition,
                        "Body Pair Cache Distance Threshold", "Maximum relative delta position for body pairs to be able to reuse collision results from last frame.")
                        ->Attribute(AZ::Edit::Attributes::Suffix, " " + Physics::NameConstants::GetLengthUnit())
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_bodyPairCacheCosMaxDeltaRotation,
                        "Body Pair Cache Rotation Threshold", "Maximum relative delta orientation for body pairs to be able to reuse collision results from last frame.")
                        ->Attribute(AZ::Edit::Attributes::Suffix, " \xC2\xB0")

                    ->ClassElement(AZ::Edit::ClassElements::Group, "Limits")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_maxBodies,
                        "MaxBodies", "Maximum bodies per physics scene.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_numBodyMutexes,
                        "NumBodyMutexes", "Number of mutexes to protect bodies from concurrent access.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_maxBodyPairs,
                        "MaxBodyPairs", "Number of body pairs that can be queued at any time.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_maxContactConstraints,
                        "MaxContactConstraints", "Number of collisions between bodies that can be handled each update.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SystemInitSettings::m_allocationArenaSize,
                        "AllocationArenaSize", "Maximum memory to allocate per physics update.")
                        ->Attribute(AZ::Edit::Attributes::Suffix, " MB")

                    ;
            }
        }
    }

    bool SystemInitSettings::operator==(const SystemInitSettings& other) const
    {
        return m_numVelocitySteps == other.m_numVelocitySteps
            && m_numPositionSteps == other.m_numPositionSteps
            && m_allowSleeping == other.m_allowSleeping
            && m_timeBeforeSleep == other.m_timeBeforeSleep
            && m_pointVelocitySleepThreshold == other.m_pointVelocitySleepThreshold
            && m_deterministicSimulation == other.m_deterministicSimulation
            && m_collisionSteps == other.m_collisionSteps
            && m_speculativeContactDistance == other.m_speculativeContactDistance
            && m_penetrationSlop == other.m_penetrationSlop
            && m_baumgarte == other.m_baumgarte
            && m_useBodyPairContactCache == other.m_useBodyPairContactCache
            && m_bodyPairCacheMaxDeltaPosition == other.m_bodyPairCacheMaxDeltaPosition
            && m_bodyPairCacheCosMaxDeltaRotation == other.m_bodyPairCacheCosMaxDeltaRotation
            && m_maxBodies == other.m_maxBodies
            && m_numBodyMutexes == other.m_numBodyMutexes
            && m_maxContactConstraints == other.m_maxContactConstraints
            && m_maxBodyPairs == other.m_maxBodyPairs
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
