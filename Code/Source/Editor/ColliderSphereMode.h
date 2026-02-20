
#pragma once

#include <AzFramework/Entity/EntityDebugDisplayBus.h>
#include <AzToolsFramework/ComponentModes/SphereViewportEdit.h>
#include <Editor/ComponentModes/JoltSubComponentModeBase.h>

namespace JoltPhysics
{
    /// Sub component mode for modifying the box dimensions on a collider.
    class ColliderSphereMode
        : public JoltSubComponentModeBase
        , private AzFramework::EntityDebugDisplayEventBus::Handler
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL

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

        void OnManipulatorMoved(const AzToolsFramework::LinearManipulator::Action& action, const AZ::EntityComponentIdPair& idPair);

        AZ::EntityComponentIdPair m_entityComponentIdPair;
        AZStd::unique_ptr<AzToolsFramework::SphereViewportEdit> m_sphereViewportEdit;
    };
} //namespace JoltPhysics
