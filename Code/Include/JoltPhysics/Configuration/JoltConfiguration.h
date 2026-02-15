
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
    //! This class holds the default values required to initialize Jolt.
    //! It also contains the default collision steps per physics update call and allocation size.
    //! The physics update requires a static buffer allocator to be set with a max allocation size.
    class SystemInitSettings
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_TYPE_INFO(SystemInitSettings, "{792779D7-C4FE-40B9-B72A-5FBDAD64352C}")

        static void Reflect(AZ::ReflectContext* context);

        //! Number of solver velocity iterations to run.
        //! Note that this needs to be >= 2 in order for friction to work (friction is applied using the non-penetration impulse from the previous iteration).
        unsigned int m_numVelocitySteps = 10;

        //! Number of solver position iterations to run.
        unsigned int m_numPositionSteps = 2;

        //! Radius around objects inside which speculative contact points will be detected. Note that if this is too big
        //! you will get ghost collisions as speculative contacts are based on the closest points during the collision detection
        //! step which may not be the actual closest points by the time the two objects hit (unit: meters).
        float m_speculativeContactDistance = 0.02f;

        //! How far bodies are allowed to sink into each other (unit: meters).
        float m_penetrationSlop = 0.02f;

        //! Baumgarte stabilization factor (how much of the position error to 'fix' in 1 update) (unit: dimensionless, 0 = nothing, 1 = 100%).
        float m_baumgarte = 0.2f;

        //! Whether to allow bodies to sleep or not.
        bool m_allowSleeping = true;

        //! Time before object is allowed to go to sleep (unit: seconds).
        float m_timeBeforeSleep = 0.5f;

        //! To detect if an object is sleeping, we use 3 points:
        //! - The center of mass.
        //! - The centers of the faces of the bounding box that are furthest away from the center.
        //! The movement of these points is tracked and if the velocity of all 3 points is lower than this value,
        //! the object is allowed to go to sleep. Must be a positive number. (unit: m/s)
        float m_pointVelocitySleepThreshold = 0.03f;

        //! By default, the simulation is deterministic, it is possible to turn this off by setting this setting to false. This will make the simulation run faster but it will no longer be deterministic.
        bool m_deterministicSimulation = true;

        //! This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
        //! Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
        unsigned int m_maxBodies = 65536;

        //! This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
        //! mutexes are cheap! (they aren't)
        unsigned int m_numBodyMutexes = 128;//for a LOT of reasons, we actually want the locks on the bodies to be quite granular.

        //! This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
        //! body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
        //! too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
        //! Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
        unsigned int m_maxBodyPairs = 65536;

        //! This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
        //! number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
        //! number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
        //! Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
        unsigned int m_maxContactConstraints = 16384;

        //! The number of collision steps for the solver to perform per update call.
        int m_collisionSteps = 1;

        //! Whether or not to use the body pair cache, which removes the need for narrow phase collision detection when orientation between two bodies didn't change.
        bool m_useBodyPairContactCache = true;

        //! Maximum relative delta position for body pairs to be able to reuse collision results from last frame (units: meter^2)
        float m_bodyPairCacheMaxDeltaPosition = 0.001f; ///< 1 mm

        //! Maximum relative delta orientation for body pairs to be able to reuse collision results from last frame, stored as cos(max angle / 2)
        float m_bodyPairCacheCosMaxDeltaRotation = 2.0f; ///< cos(2 degrees)

        //! Maximum memory allocation for static buffer used during each physics update call.
        //! Used during PhysicsSystem::Initialize
        //! Value is in MB and converted to binary bytes (1024 * 1024).
        unsigned int m_allocationArenaSize = 32;

        bool operator==(const SystemInitSettings& other) const;
        bool operator!=(const SystemInitSettings& other) const;
    };

    //! Container class that holds Jolt-specific system and scene configuration values.
    struct JoltSystemConfiguration : public AzPhysics::SystemConfiguration
    {
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_RTTI(JoltPhysics::JoltSystemConfiguration, "{C7B81915-F277-4286-8D26-46739260423D}")

        static void Reflect(AZ::ReflectContext* context);

        static JoltSystemConfiguration CreateDefault();

        SystemInitSettings m_systemInitSettings;

        bool operator==(const JoltSystemConfiguration& other) const;
        bool operator!=(const JoltSystemConfiguration& other) const;
    };
}
