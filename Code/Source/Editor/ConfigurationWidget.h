
#pragma once

#if !defined(Q_MOC_RUN)
#include <JoltPhysics/Configuration/JoltConfiguration.h>
#include <Editor/ConfigurationWindowBus.h>
#include <AzFramework/Physics/Configuration/CollisionConfiguration.h>
#include <AzFramework/Physics/Configuration/SceneConfiguration.h>
#endif

#include <QWidget>

namespace AzQtComponents
{
    class TabWidget;
}

namespace JoltPhysics
{
    namespace Editor
    {
        class SettingsWidget;
        class CollisionFilteringWidget;
        // class PvdWidget;

        /// Widget for editing Jolt configuration and settings.
        ///
        class ConfigurationWidget
            : public QWidget
            , public ConfigurationWindowRequestBus::Handler
        {
            Q_OBJECT

        public:
            AZ_CLASS_ALLOCATOR(ConfigurationWidget, AZ::SystemAllocator);

            explicit ConfigurationWidget(QWidget* parent = nullptr);
            ~ConfigurationWidget() override;

            void SetConfiguration(const JoltPhysics::JoltSystemConfiguration& joltSystemConfiguration,
                                  const JoltPhysics::Debug::DebugConfiguration& joltDebugConfiguration,
                                  const AzPhysics::SceneConfiguration& defaultSceneConfiguration);

            // ConfigurationWindowRequestBus
            void ShowCollisionLayersTab() override;
            void ShowCollisionGroupsTab() override;
            void ShowGlobalSettingsTab() override;

            signals:
                void onConfigurationChanged(const JoltPhysics::JoltSystemConfiguration& joltSystemConfiguration,
                                            const JoltPhysics::Debug::DebugConfiguration& joltDebugConfig,
                                            const AzPhysics::SceneConfiguration& defaultSceneConfiguration);

        private:
            AzPhysics::SceneConfiguration m_defaultSceneConfiguration;
            JoltPhysics::JoltSystemConfiguration m_joltSystemConfiguration;
            JoltPhysics::Debug::DebugConfiguration m_joltDebugConfiguration;

            AzQtComponents::TabWidget* m_tabs;
            SettingsWidget* m_settings;
            CollisionFilteringWidget* m_collisionFiltering;
            // PvdWidget* m_pvd;
        };
    }
}
