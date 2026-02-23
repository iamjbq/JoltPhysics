#pragma once

#include <AzCore/IO/Path/Path.h>

#include <Configuration/JoltSettingsRegistryManager.h>

namespace JoltPhysics
{
    //! Handles loading and saving the settings registry
    class JoltEditorSettingsRegistryManager : public JoltSettingsRegistryManager
    {
    public:
        JoltEditorSettingsRegistryManager();

        // JoltSystemSettingsRegistry ...
        void SaveSystemConfiguration(const JoltSystemConfiguration& config, const OnJoltConfigSaveComplete& saveCallback) const override;
        void SaveDefaultSceneConfiguration(const AzPhysics::SceneConfiguration& config, const OnDefaultSceneConfigSaveComplete& saveCallback) const override;
        void SaveDebugConfiguration(const Debug::DebugConfiguration& config, const OnJoltDebugConfigSaveComplete& saveCallback) const override;

    private:
        AZ::IO::FixedMaxPath m_joltConfigurationFilePath = "Registry/joltsystemconfiguration.setreg";
        AZ::IO::FixedMaxPath m_defaultSceneConfigFilePath = "Registry/joltdefaultsceneconfiguration.setreg";
        AZ::IO::FixedMaxPath m_debugConfigurationFilePath = "Registry/joltdebugconfiguration.setreg";

        bool m_initialized = false;
    };
} // JoltPhysics

