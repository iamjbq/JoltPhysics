#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Collision/CollisionGroups.h>
#include <AzFramework/Physics/Collision/CollisionLayers.h>
#include <AzToolsFramework/UI/PropertyEditor/ReflectedPropertyEditor.hxx>
#include <AzToolsFramework/UI/PropertyEditor/InstanceDataHierarchy.h>
#include <AzQtComponents/Components/Widgets/TabWidget.h>
#include <QBoxLayout>
#include <Editor/ConfigurationWidget.h>
#include <Editor/SettingsWidget.h>
#include <Editor/CollisionFilteringWidget.h>
#include <Editor/ConfigurationWidget.h>

namespace JoltPhysics
{
    namespace Editor
    {
        ConfigurationWidget::ConfigurationWidget(QWidget* parent)
            : QWidget(parent)
        {
            QVBoxLayout* verticalLayout = new QVBoxLayout(this);
            verticalLayout->setContentsMargins(0, 5, 0, 0);
            verticalLayout->setSpacing(0);

            m_tabs = new AzQtComponents::TabWidget(this);
            AzQtComponents::TabWidget::applySecondaryStyle(m_tabs, false);

            m_settings = new SettingsWidget();
            m_collisionFiltering = new CollisionFilteringWidget();
            // m_pvd = new PvdWidget();

            m_tabs->addTab(m_settings, "Global Configuration");
            m_tabs->addTab(m_collisionFiltering, "Collision Filtering");
            // m_tabs->addTab(m_pvd, "Debugger");

            verticalLayout->addWidget(m_tabs);

            connect(m_settings, &SettingsWidget::onValueChanged,
                this, [this](const JoltPhysics::JoltSystemConfiguration& joltSystemConfiguration,
                            const AzPhysics::SceneConfiguration& defaultSceneConfiguration,
                             const Debug::DebugDisplayData& debugDisplayData
                             )
            {
                m_joltSystemConfiguration = joltSystemConfiguration;
                m_defaultSceneConfiguration = defaultSceneConfiguration;
                m_joltDebugConfiguration.m_debugDisplayData = debugDisplayData;
                emit onConfigurationChanged(m_joltSystemConfiguration,
                    m_joltDebugConfiguration,
                    m_defaultSceneConfiguration);
            });

            connect(m_collisionFiltering, &CollisionFilteringWidget::onConfigurationChanged,
                this, [this](const AzPhysics::CollisionLayers& layers, const AzPhysics::CollisionGroups& groups)
            {
                m_joltSystemConfiguration.m_collisionConfig.m_collisionLayers = layers;
                m_joltSystemConfiguration.m_collisionConfig.m_collisionGroups = groups;
                emit onConfigurationChanged(m_joltSystemConfiguration,
                    m_joltDebugConfiguration,
                    m_defaultSceneConfiguration);
            });

            // connect(m_pvd, &PvdWidget::onValueChanged,
            //     this, [this](const Debug::PvdConfiguration& configuration)
            // {
            //     m_joltDebugConfiguration.m_pvdConfigurationData = configuration;
            //     emit onConfigurationChanged(m_joltSystemConfiguration, m_joltDebugConfiguration, m_defaultSceneConfiguration);
            // });

            ConfigurationWindowRequestBus::Handler::BusConnect();
        }

        ConfigurationWidget::~ConfigurationWidget()
        {
            ConfigurationWindowRequestBus::Handler::BusDisconnect();
        }

        void ConfigurationWidget::SetConfiguration(
            const JoltPhysics::JoltSystemConfiguration& joltSystemConfiguration,
            const JoltPhysics::Debug::DebugConfiguration& joltDebugConfiguration,
            const AzPhysics::SceneConfiguration& defaultSceneConfiguration)
        {
            m_joltSystemConfiguration = joltSystemConfiguration;
            m_defaultSceneConfiguration = defaultSceneConfiguration;
            m_joltDebugConfiguration = joltDebugConfiguration;
            m_settings->SetValue(m_joltSystemConfiguration,
                m_defaultSceneConfiguration,
                m_joltDebugConfiguration.m_debugDisplayData
                );
            m_collisionFiltering->SetConfiguration(m_joltSystemConfiguration.m_collisionConfig.m_collisionLayers, m_joltSystemConfiguration.m_collisionConfig.m_collisionGroups);
            // m_pvd->SetValue(m_joltDebugConfiguration.m_pvdConfigurationData);
        }

        void ConfigurationWidget::ShowCollisionLayersTab()
        {
            const int index = m_tabs->indexOf(m_collisionFiltering);
            m_tabs->setCurrentIndex(index);
            m_collisionFiltering->ShowLayersTab();
        }

        void ConfigurationWidget::ShowCollisionGroupsTab()
        {
            const int index = m_tabs->indexOf(m_collisionFiltering);
            m_tabs->setCurrentIndex(index);
            m_collisionFiltering->ShowGroupsTab();
        }

        void ConfigurationWidget::ShowGlobalSettingsTab()
        {
            const int index = m_tabs->indexOf(m_settings);
            m_tabs->setCurrentIndex(index);
        }
    }
}

#include <Editor/moc_ConfigurationWidget.cpp>
