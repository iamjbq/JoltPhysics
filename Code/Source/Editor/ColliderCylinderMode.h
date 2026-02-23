
#pragma once

#include <AzFramework/Entity/EntityDebugDisplayBus.h>
#include <AzToolsFramework/ComponentModes/CapsuleViewportEdit.h>
#include <Editor/ComponentModes/JoltSubComponentModeBase.h>

namespace JoltPhysics
{
    //! Sub component mode for modifying the height and radius on a cylinder collider.
    class ColliderCylinderMode
        : public JoltSubComponentModeBase
        , private AzFramework::EntityDebugDisplayEventBus::Handler
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL

        // JoltSubComponentModeBase overrides ...
        void Setup(const AZ::EntityComponentIdPair& idPair) override;
        void Refresh(const AZ::EntityComponentIdPair& idPair) override;
        void Teardown(const AZ::EntityComponentIdPair& idPair) override;
        void ResetValues(const AZ::EntityComponentIdPair& idPair) override;

    private:
        // AzFramework::EntityDebugDisplayEventBus overrides ...
        void DisplayEntityViewport(
            const AzFramework::ViewportInfo& viewportInfo,
            AzFramework::DebugDisplayRequests& debugDisplay) override;

        AZ::EntityComponentIdPair m_entityComponentIdPair;

        // cylinder is equivalent to capsule for the purposes of viewport editing
        AZStd::unique_ptr<AzToolsFramework::CapsuleViewportEdit> m_capsuleViewportEdit;
    };
} //namespace JoltPhysics
