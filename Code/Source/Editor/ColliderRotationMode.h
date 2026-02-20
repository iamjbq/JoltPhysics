
#pragma once

#include <AzToolsFramework/Manipulators/RotationManipulators.h>
#include <AzFramework/Entity/EntityDebugDisplayBus.h>
#include <Editor/ComponentModes/JoltSubComponentModeBase.h>

namespace JoltPhysics
{
    /// Sub component mode for modifying the rotation on a collider in the viewport.
    class ColliderRotationMode
        : public JoltSubComponentModeBase
        , private AzFramework::EntityDebugDisplayEventBus::Handler
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL

        ColliderRotationMode();

        // JoltSubComponentModeBase ...
        void Setup(const AZ::EntityComponentIdPair& idPair) override;
        void Refresh(const AZ::EntityComponentIdPair& idPair) override;
        void Teardown(const AZ::EntityComponentIdPair& idPair) override;
        void ResetValues(const AZ::EntityComponentIdPair& idPair) override;

    private:
        // AzFramework::EntityDebugDisplayEventBus ...
        void DisplayEntityViewport(
            const AzFramework::ViewportInfo& viewportInfo,
            AzFramework::DebugDisplayRequests& debugDisplay) override;

        AzToolsFramework::RotationManipulators m_rotationManipulators;
    };
} //namespace JoltPhysics
