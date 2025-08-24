
#include <AzCore/Interface/Interface.h>
#include <AzFramework/Physics/CollisionBus.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Configuration/CollisionConfiguration.h>
#include <AzFramework/Physics/Configuration/SceneConfiguration.h>
#include <AzToolsFramework/API/ViewPaneOptions.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
// #include <LyViewPaneNames.h>

// #include <Editor/ui_EditorWindow.h>
#include <Editor/EditorWindow.h>
// #include <Editor/ConfigurationWidget.h>
#include <System/JoltPhysicsSystem.h>
#include <JoltPhysics/Configuration/JoltSystemConfiguration.h>

namespace  JoltPhysics
{
    namespace Editor
    {
        EditorWindow::EditorWindow(QWidget* parent)
            : QWidget(parent)
            , m_ui(new Ui::EditorWindowClass())
        {
            m_ui->setupUi(this);

            auto* physicsSystem = AZ::Interface<AzPhysics::SystemInterface>::Get();
            const auto* JoltSys = azdynamic_cast<const JoltPhysics::JoltSystemConfiguration*>(physicsSystem->GetConfiguration());
            const AzPhysics::SceneConfiguration& defaultSceneConfiguration = physicsSystem->GetDefaultSceneConfiguration();
            // const JoltPhysics::Debug::DebugConfiguration& physXDebugConfiguration = AZ::Interface<JoltPhysics::Debug::PhysXDebugInterface>::Get()->GetDebugConfiguration();
            

            m_ui->m_JoltConfigurationWidget->SetConfiguration(*JoltSystemConfiguration, JoltDebugConfiguration, defaultSceneConfiguration);
            connect(m_ui->m_PhysXConfigurationWidget, &PhysX::Editor::ConfigurationWidget::onConfigurationChanged, 
                this, &EditorWindow::SaveConfiguration);
        }

        void EditorWindow::RegisterViewClass()
        {
            AzToolsFramework::ViewPaneOptions options;
            options.preferedDockingArea = Qt::LeftDockWidgetArea;
            options.saveKeyName = "JOltConfiguration";
            options.isPreview = true;
            AzToolsFramework::RegisterViewPane<EditorWindow>(LyViewPane::JoltConfigurationEditor, LyViewPane::CategoryTools, options);
        }

        void EditorWindow::SaveConfiguration(
            const JoltPhysics::JoltSystemConfiguration& physXSystemConfiguration,
            const JoltPhysics::Debug::DebugConfiguration& physXDebugConfig,
            const AzPhysics::SceneConfiguration& defaultSceneConfiguration)
        {
            auto* joltSystem = GetPhysXSystem();
            if (joltSystem == nullptr)
            {
                AZ_Error("JoltPhysics", false, "Unable to save the Jolt configuration. The JoltSystem not initialized. Any changes have not been applied.");
                return;
            }

            //update the Jolt system config if it has changed
            const JoltSettingsRegistryManager& settingsRegManager = joltSystem->GetSettingsRegistryManager();
            if (joltSystem->GetPhysXConfiguration() != physXSystemConfiguration)
            {
                auto saveCallback = [](const PhysXSystemConfiguration& config, PhysXSettingsRegistryManager::Result result)
                {
                    AZ_Warning("PhysX", result == PhysXSettingsRegistryManager::Result::Success, "Unable to save the PhysX configuration. Any changes have not been applied.");
                    if (result == PhysXSettingsRegistryManager::Result::Success)
                    {
                        if (auto* physXSystem = GetPhysXSystem())
                        {
                            physXSystem->UpdateConfiguration(&config);
                        }
                    }
                };
                settingsRegManager.SaveSystemConfiguration(physXSystemConfiguration, saveCallback);
            }

            if (joltSystem->GetDefaultSceneConfiguration() != defaultSceneConfiguration)
            {
                auto saveCallback = [](const AzPhysics::SceneConfiguration& config, PhysXSettingsRegistryManager::Result result)
                {
                    AZ_Warning("PhysX", result == PhysXSettingsRegistryManager::Result::Success, "Unable to save the Default Scene configuration. Any changes have not been applied.");
                    if (result == PhysXSettingsRegistryManager::Result::Success)
                    {
                        if (auto* physXSystem = GetPhysXSystem())
                        {
                            physXSystem->UpdateDefaultSceneConfiguration(config);
                        }
                    }
                };
                settingsRegManager.SaveDefaultSceneConfiguration(defaultSceneConfiguration, saveCallback);
            }

            //Update the debug configuration
            if (auto* physXDebug = AZ::Interface<Debug::PhysXDebugInterface>::Get())
            {
                if (physXDebug->GetDebugConfiguration() != physXDebugConfig)
                {
                    auto saveCallback = [](const Debug::DebugConfiguration& config, PhysXSettingsRegistryManager::Result result)
                    {
                        AZ_Warning("PhysX", result == PhysXSettingsRegistryManager::Result::Success, "Unable to save the PhysX debug configuration. Any changes have not been applied.");
                        if (result == PhysXSettingsRegistryManager::Result::Success)
                        {
                            if (auto* physXDebug = AZ::Interface<Debug::PhysXDebugInterface>::Get())
                            {
                                physXDebug->UpdateDebugConfiguration(config);
                            }
                        }
                    };
                    settingsRegManager.SaveDebugConfiguration(physXDebugConfig, saveCallback);
                }
            }
        }
    }
}
