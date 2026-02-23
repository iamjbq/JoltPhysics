
#include <AzCore/Interface/Interface.h>
#include <AzFramework/Physics/CollisionBus.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Configuration/CollisionConfiguration.h>
#include <AzFramework/Physics/Configuration/SceneConfiguration.h>
#include <AzToolsFramework/API/ViewPaneOptions.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <LyViewPaneNames.h>

#include <Editor/ui_EditorWindow.h>
#include <Editor/EditorWindow.h>
#include <Editor/ConfigurationWidget.h>
#include <System/JoltSystem.h>
#include <JoltPhysics/Configuration/JoltConfiguration.h>
#include "EditorWindow.h"

namespace JoltPhysics
{
    namespace Editor
    {
        EditorWindow::EditorWindow(QWidget* parent)
            : QWidget(parent)
            , m_ui(new Ui::EditorWindowClass())
        {
            m_ui->setupUi(this);

            auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get();
            const auto* joltSystemConfiguration = azdynamic_cast<const JoltPhysics::JoltSystemConfiguration*>(physicsSystem->GetConfiguration());
            const AzPhysics::SceneConfiguration& defaultSceneConfiguration = physicsSystem->GetDefaultSceneConfiguration();
            const JoltPhysics::Debug::DebugConfiguration& joltDebugConfiguration = AZ::Interface<JoltPhysics::Debug::JoltDebugInterface>::Get()->GetDebugConfiguration();
            

            m_ui->m_joltConfigurationWidget->SetConfiguration(
                *joltSystemConfiguration,
                joltDebugConfiguration,
                defaultSceneConfiguration
                );
            connect(m_ui->m_joltConfigurationWidget, &JoltPhysics::Editor::ConfigurationWidget::onConfigurationChanged,
                this, &EditorWindow::SaveConfiguration);
        }

        EditorWindow::~EditorWindow()
        {
        }
        
        void EditorWindow::RegisterViewClass()
        {
            AzToolsFramework::ViewPaneOptions options;
            options.preferedDockingArea = Qt::LeftDockWidgetArea;
            options.saveKeyName = "JoltConfiguration";
            options.isPreview = true;
            AzToolsFramework::RegisterViewPane<EditorWindow>(LyViewPane::JoltConfigurationEditor, LyViewPane::CategoryTools, options);
        }

        void EditorWindow::SaveConfiguration(
            const JoltPhysics::JoltSystemConfiguration& joltSystemConfiguration,
            const JoltPhysics::Debug::DebugConfiguration& joltDebugConfig,
            const AzPhysics::SceneConfiguration& defaultSceneConfiguration)
        {
            auto* joltSystem = GetJoltSystem();
            if (joltSystem == nullptr)
            {
                AZ_Error("JoltPhysics", false, "Unable to save the Jolt configuration. The JoltSystem is not initialized. Any changes have not been applied.");
                return;
            }

            //update the Jolt system config if it has changed
            const JoltSettingsRegistryManager& settingsRegManager = joltSystem->GetSettingsRegistryManager();
            if (joltSystem->GetJoltConfiguration() != joltSystemConfiguration)
            {
                auto saveCallback = [](const JoltSystemConfiguration& config, JoltSettingsRegistryManager::Result result)
                {
                    AZ_Warning("Jolt", result == JoltSettingsRegistryManager::Result::Success, "Unable to save the Jolt configuration. Any changes have not been applied.");
                    if (result == JoltSettingsRegistryManager::Result::Success)
                    {
                        if (auto* joltSystem = GetJoltSystem())
                        {
                            joltSystem->UpdateConfiguration(&config);
                        }
                    }
                };
                settingsRegManager.SaveSystemConfiguration(joltSystemConfiguration, saveCallback);
            }

            if (joltSystem->GetDefaultSceneConfiguration() != defaultSceneConfiguration)
            {
                auto saveCallback = [](const AzPhysics::SceneConfiguration& config, JoltSettingsRegistryManager::Result result)
                {
                    AZ_Warning("Jolt", result == JoltSettingsRegistryManager::Result::Success, "Unable to save the Default Scene configuration. Any changes have not been applied.");
                    if (result == JoltSettingsRegistryManager::Result::Success)
                    {
                        if (auto* joltSystem = GetJoltSystem())
                        {
                            joltSystem->UpdateDefaultSceneConfiguration(config);
                        }
                    }
                };
                settingsRegManager.SaveDefaultSceneConfiguration(defaultSceneConfiguration, saveCallback);
            }

            //Update the debug configuration
            if (auto* joltDebug = AZ::Interface<Debug::JoltDebugInterface>::Get())
            {
                if (joltDebug->GetDebugConfiguration() != joltDebugConfig)
                {
                    auto saveCallback = [](const Debug::DebugConfiguration& config, JoltSettingsRegistryManager::Result result)
                    {
                        AZ_Warning("Jolt", result == JoltSettingsRegistryManager::Result::Success, "Unable to save the Jolt debug configuration. Any changes have not been applied.");
                        if (result == JoltSettingsRegistryManager::Result::Success)
                        {
                            if (auto* joltDebug = AZ::Interface<Debug::JoltDebugInterface>::Get())
                            {
                                joltDebug->UpdateDebugConfiguration(config);
                            }
                        }
                    };
                    settingsRegManager.SaveDebugConfiguration(joltDebugConfig, saveCallback);
                }
            }
        }
    }
}
