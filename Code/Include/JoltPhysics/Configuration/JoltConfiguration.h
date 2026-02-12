
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
