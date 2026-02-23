#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzToolsFramework/UI/PropertyEditor/ReflectedPropertyEditor.hxx>
#include <AzToolsFramework/UI/PropertyEditor/InstanceDataHierarchy.h>
#include <QBoxLayout>
#include <Editor/SettingsWidget.h>
#include <Editor/DocumentationLinkWidget.h>
#include <NameConstants.h>

namespace JoltPhysics
{
    namespace Editor
    {
        static const char* const s_settingsDocumentationLink = "Learn more about <a href=%0>configuring Jolt.</a>";
        static const char* const s_settingsDocumentationAddress = "configuring/configuration-global";

        SettingsWidget::SettingsWidget(QWidget* parent)
            : QWidget(parent)
        {
            CreatePropertyEditor(this);
        }

        void SettingsWidget::SetValue(const JoltPhysics::JoltSystemConfiguration& joltSystemConfiguration,
            const AzPhysics::SceneConfiguration& defaultSceneConfiguration,
            const Debug::DebugDisplayData& debugDisplayData
            )
        {
            m_joltSystemConfiguration = joltSystemConfiguration;
            m_defaultSceneConfiguration = defaultSceneConfiguration;
            m_debugDisplayData = debugDisplayData;

            blockSignals(true);
            m_propertyEditor->ClearInstances();
            m_propertyEditor->AddInstance(&m_joltSystemConfiguration.m_systemInitSettings);
            m_propertyEditor->AddInstance(&m_joltSystemConfiguration);
            m_propertyEditor->AddInstance(&m_defaultSceneConfiguration);
            m_propertyEditor->AddInstance(&m_debugDisplayData);
            m_propertyEditor->InvalidateAll();
            blockSignals(false);
        }

        void SettingsWidget::CreatePropertyEditor(QWidget* parent)
        {
            QVBoxLayout* verticalLayout = new QVBoxLayout(parent);
            verticalLayout->setContentsMargins(0, 0, 0, 0);
            verticalLayout->setSpacing(0);

            m_documentationLinkWidget = new DocumentationLinkWidget(s_settingsDocumentationLink, (UXNameConstants::GetJoltDocsRoot() + s_settingsDocumentationAddress).c_str());

            AZ::SerializeContext* m_serializeContext;
            AZ::ComponentApplicationBus::BroadcastResult(m_serializeContext, &AZ::ComponentApplicationRequests::GetSerializeContext);
            AZ_Assert(m_serializeContext, "Failed to retrieve serialize context.");

            const int propertyLabelWidth = 250;
            m_propertyEditor = new AzToolsFramework::ReflectedPropertyEditor(parent);
            m_propertyEditor->Setup(m_serializeContext, this, true, propertyLabelWidth);
            m_propertyEditor->show();
            m_propertyEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            verticalLayout->addWidget(m_documentationLinkWidget);
            verticalLayout->addWidget(m_propertyEditor);
        }

        void SettingsWidget::BeforePropertyModified(AzToolsFramework::InstanceDataNode* /*node*/)
        {
        }

        void SettingsWidget::AfterPropertyModified(AzToolsFramework::InstanceDataNode* /*node*/)
        {
        }

        void SettingsWidget::SetPropertyEditingActive(AzToolsFramework::InstanceDataNode* /*node*/)
        {
        }

        void SettingsWidget::SetPropertyEditingComplete(AzToolsFramework::InstanceDataNode* /*node*/)
        {
            emit onValueChanged(m_joltSystemConfiguration,
                m_defaultSceneConfiguration,
                m_debugDisplayData
            );
        }

        void SettingsWidget::SealUndoStack()
        {
        }
    } // Editor
} // JoltPhysics

#include <Editor/moc_SettingsWidget.cpp>
