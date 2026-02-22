#pragma once

#if !defined(Q_MOC_RUN)
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI_Internals.h>
#include <AzToolsFramework/UI/PropertyEditor/ReflectedPropertyEditor.hxx>
#include <AzFramework/Physics/Configuration/SceneConfiguration.h>
#include <JoltPhysics/Configuration/JoltConfiguration.h>
#endif

#include <QWidget>
#include <JoltPhysics/Debug/JoltDebugInterface.h>

namespace JoltPhysics
{
    namespace Editor
    {
        class DocumentationLinkWidget;

        class SettingsWidget
            : public QWidget
            , private AzToolsFramework::IPropertyEditorNotify
        {
            Q_OBJECT

        public:
            AZ_CLASS_ALLOCATOR(SettingsWidget, AZ::SystemAllocator);

            explicit SettingsWidget(QWidget* parent = nullptr);

            void SetValue(const JoltPhysics::JoltSystemConfiguration& joltSystemConfiguration,
                const AzPhysics::SceneConfiguration& defaultSceneConfiguration,
                const Debug::DebugDisplayData& debugDisplayData
                );

        signals:
            void onValueChanged(const JoltPhysics::JoltSystemConfiguration& joltSystemConfiguration,
                const AzPhysics::SceneConfiguration& defaultSceneConfiguration,
                const JoltPhysics::Debug::DebugDisplayData& debugDisplayData
                );

        private:
            void CreatePropertyEditor(QWidget* parent);

            void BeforePropertyModified(AzToolsFramework::InstanceDataNode* /*node*/) override;
            void AfterPropertyModified(AzToolsFramework::InstanceDataNode* /*node*/) override;
            void SetPropertyEditingActive(AzToolsFramework::InstanceDataNode* /*node*/) override;
            void SetPropertyEditingComplete(AzToolsFramework::InstanceDataNode* /*node*/) override;
            void SealUndoStack() override;

            AzToolsFramework::ReflectedPropertyEditor* m_propertyEditor;
            DocumentationLinkWidget* m_documentationLinkWidget;
            JoltPhysics::JoltSystemConfiguration m_joltSystemConfiguration;
            AzPhysics::SceneConfiguration m_defaultSceneConfiguration;
            JoltPhysics::Debug::DebugDisplayData m_debugDisplayData;
        };
    }
}
