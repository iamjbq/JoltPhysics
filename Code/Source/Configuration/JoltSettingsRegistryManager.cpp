#include "JoltSettingsRegistryManager.h"

#include <AzCore/Settings/SettingsRegistry.h>
#include <AzCore/Settings/SettingsRegistryMergeUtils.h>

#include <AzFramework/Physics/Configuration/CollisionConfiguration.h>

namespace JoltPhysics
{
#if !defined(JOLT_SETREG_GEM_NAME)
    #error "Missing required JOLT_SETREG_GEM_NAME definition"
#endif //!defined(JOLT_SETREG_GEM_NAME)

    JoltSettingsRegistryManager::JoltSettingsRegistryManager()
    {
        m_settingsRegistryPath = AZStd::string::format("%s/Gems/" JOLT_SETREG_GEM_NAME "/JoltSystemConfiguration", AZ::SettingsRegistryMergeUtils::OrganizationRootKey);
        m_defaultSceneConfigSettingsRegistryPath = AZStd::string::format("%s/Gems/" JOLT_SETREG_GEM_NAME "/DefaultSceneConfiguration", AZ::SettingsRegistryMergeUtils::OrganizationRootKey);
        // m_debugSettingsRegistryPath = AZStd::string::format("%s/Gems/" JOLT_SETREG_GEM_NAME "/Debug/PhysXDebugConfiguration", AZ::SettingsRegistryMergeUtils::OrganizationRootKey);
    }

    AZStd::optional<JoltSystemConfiguration> JoltSettingsRegistryManager::LoadSystemConfiguration() const
    {
        JoltSystemConfiguration systemConfig;

        bool configurationRead = false;
        
        AZ::SettingsRegistryInterface* settingsRegistry = AZ::SettingsRegistry::Get();
        if (settingsRegistry)
        {
            configurationRead = settingsRegistry->GetObject(systemConfig, m_settingsRegistryPath);
        }

        if (configurationRead)
        {
            AZ_TracePrintf("JoltSystem", R"(JoltConfiguration was read from settings registry at pointer path)"
                R"( "%s)" "\n",
                m_settingsRegistryPath.c_str());
            return systemConfig;
        }
        return AZStd::nullopt;
    }

    AZStd::optional<AzPhysics::SceneConfiguration> JoltSettingsRegistryManager::LoadDefaultSceneConfiguration() const
    {
        AzPhysics::SceneConfiguration sceneConfig;
        bool configurationRead = false;

        AZ::SettingsRegistryInterface* settingsRegistry = AZ::SettingsRegistry::Get();
        if (settingsRegistry)
        {
            configurationRead = settingsRegistry->GetObject(sceneConfig, m_defaultSceneConfigSettingsRegistryPath);
        }

        if (configurationRead)
        {
            AZ_TracePrintf("JoltSystem", R"(Default Scene Configuration was read from settings registry at pointer path)"
                R"("%s)" "\n",
                m_defaultSceneConfigSettingsRegistryPath.c_str());
            return sceneConfig;
        }
        return AZStd::nullopt;
    }

    // AZStd::optional<Debug::DebugConfiguration> JoltSettingsRegistryManager::LoadDebugConfiguration() const
    // {
    //     Debug::DebugConfiguration systemConfig;
    //
    //     bool configurationRead = false;
    //
    //     AZ::SettingsRegistryInterface* settingsRegistry = AZ::SettingsRegistry::Get();
    //     if (settingsRegistry)
    //     {
    //         configurationRead = settingsRegistry->GetObject(systemConfig, m_debugSettingsRegistryPath);
    //     }
    //
    //     if (configurationRead)
    //     {
    //         AZ_TracePrintf("PhysXSystem", R"(Debug::DebugConfiguration was read from settings registry at pointer path)"
    //             R"( "%s)" "\n",
    //             m_debugSettingsRegistryPath.c_str());
    //         return systemConfig;
    //     }
    //     return AZStd::nullopt;
    // }

    void JoltSettingsRegistryManager::SaveSystemConfiguration([[maybe_unused]] const JoltSystemConfiguration& config, const OnJoltConfigSaveComplete& saveCallback) const
    {
        //PhysXEditorSettingsRegistryManager will implement, as saving is editor only currently.
        if (saveCallback)
        {
            saveCallback(config, Result::Failed);
        } 
    }

    void JoltSettingsRegistryManager::SaveDefaultSceneConfiguration([[maybe_unused]] const AzPhysics::SceneConfiguration& config, const OnDefaultSceneConfigSaveComplete& saveCallback) const
    {
        //PhysXEditorSettingsRegistryManager will implement, as saving is editor only currently.
        if (saveCallback)
        {
            saveCallback(config, Result::Failed);
        }
    }

    // void JoltSettingsRegistryManager::SaveDebugConfiguration([[maybe_unused]] const Debug::DebugConfiguration& config, const OnPhysXDebugConfigSaveComplete& saveCallback) const
    // {
    //     //PhysXEditorSettingsRegistryManager will implement, as saving is editor only currently.
    //     if (saveCallback)
    //     {
    //         saveCallback(config, Result::Failed);
    //     }
    // }
} // JoltPhysics
