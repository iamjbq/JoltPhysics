
#pragma once

#if !defined(Q_MOC_RUN)
#include <AzCore/Asset/AssetCommon.h>
#include <QWidget>
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

namespace JoltPhysics
{
    struct JoltSystemConfiguration;
    // namespace Debug
    // {
    //     struct DebugConfiguration;
    // }

    namespace Editor
    {
        /// Window pane wrapper for the PhysX Configuration Widget.
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
