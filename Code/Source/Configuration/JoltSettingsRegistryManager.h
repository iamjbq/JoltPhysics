#pragma once

#include <AzCore/std/optional.h>
#include <AzFramework/Physics/Configuration/SceneConfiguration.h>
#include <JoltPhysics/Configuration/JoltConfiguration.h>
#include <JoltPhysics/Debug/JoltDebugConfiguration.h>

namespace JoltPhysics
{
    //! Handles loading and saving the settings registry
    class JoltSettingsRegistryManager
    {
        public:
        enum class Result : AZ::u8
        {
            Success,
            Failed
        };
        using OnJoltConfigSaveComplete = AZStd::function<void(const JoltSystemConfiguration&, Result)>;
        using OnDefaultSceneConfigSaveComplete = AZStd::function<void(const AzPhysics::SceneConfiguration&, Result)>;
        using OnJoltDebugConfigSaveComplete = AZStd::function<void(const Debug::DebugConfiguration&, Result)>;

        JoltSettingsRegistryManager();
        virtual ~JoltSettingsRegistryManager() = default;

        //! Load the Jolt Configuration from the Settings Registry
        //! @return Returns true if successful.
        virtual AZStd::optional<JoltSystemConfiguration> LoadSystemConfiguration() const;

        //! Load the Default Scene Configuration from the Settings Registry
        //! @return Returns true if successful.
        virtual AZStd::optional<AzPhysics::SceneConfiguration> LoadDefaultSceneConfiguration() const;

        //! Load the Jolt Debug Configuration from the Settings Registry
        //! @return Returns true if successful.
        virtual AZStd::optional<Debug::DebugConfiguration> LoadDebugConfiguration() const;

        //! Save the Jolt Configuration from the Settings Registry
        //! @return Returns true if successful. When not in Editor, always returns false.
        virtual void SaveSystemConfiguration(const JoltSystemConfiguration& config, const OnJoltConfigSaveComplete& saveCallback) const;

        //! Save the Default Scene Configuration  from the Settings Registry
        //! @return Returns true if successful. When not in Editor, always returns false.
        virtual void SaveDefaultSceneConfiguration(const AzPhysics::SceneConfiguration& config, const OnDefaultSceneConfigSaveComplete& saveCallback) const;

        //! Save the Jolt Debug Configuration from the Settings Registry
        //! @return Returns true if successful. When not in Editor, always returns false.
        virtual void SaveDebugConfiguration(const Debug::DebugConfiguration& config, const OnJoltDebugConfigSaveComplete& saveCallback) const;

    protected:
        AZStd::string m_settingsRegistryPath;
        AZStd::string m_defaultSceneConfigSettingsRegistryPath;
        AZStd::string m_debugSettingsRegistryPath;
    };
} // JoltPhysics
