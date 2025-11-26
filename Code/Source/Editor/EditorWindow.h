
#pragma once

#if !defined(Q_MOC_RUN)
#include <AzCore/Asset/AssetCommon.h>
#include <QWidget>
#include <QScopedPointer>
#endif

namespace AzPhysics
{
    class CollisionConfiguration;
    struct SceneConfiguration;
}

namespace Ui
{
    class EditorWindowClass;
}

// This is defined in the core engine editor, so gonna try this first
namespace LyViewPane
{
    static const char* const JoltConfigurationEditor = "Jolt Configuration";
}

namespace JoltPhysics
{
    struct JoltSystemConfiguration;
    // namespace Debug
    // {
    //     struct DebugConfiguration;
    // }

    namespace Editor
    {
        /// Window pane wrapper for the Jolt Configuration Widget.
        ///
        class EditorWindow
            : public QWidget
        {
            Q_OBJECT
        public:
            AZ_CLASS_ALLOCATOR(EditorWindow, AZ::SystemAllocator);
            static void RegisterViewClass();

            explicit EditorWindow(QWidget* parent = nullptr);

        private:
            static void SaveConfiguration(
                const JoltPhysics::JoltSystemConfiguration& joltSystemConfiguration,
                // const JoltPhysics::Debug::DebugConfiguration& joltDebugConfiguration,
                const AzPhysics::SceneConfiguration& defaultSceneConfiguration);

            QScopedPointer<Ui::EditorWindowClass> m_ui;
        };
    }
};
