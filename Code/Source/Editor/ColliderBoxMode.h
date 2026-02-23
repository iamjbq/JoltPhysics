
#pragma once

#include <AzToolsFramework/ComponentModes/BoxViewportEdit.h>
#include <Editor/ComponentModes/JoltSubComponentModeBase.h>

namespace JoltPhysics
{
    /// Sub component mode for modifying the box dimensions on a collider.
    class ColliderBoxMode : public JoltSubComponentModeBase
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL

        ColliderBoxMode();

        // JoltSubComponentModeBase ...
        void Setup(const AZ::EntityComponentIdPair& idPair) override;
        void Refresh(const AZ::EntityComponentIdPair& idPair) override;
        void Teardown(const AZ::EntityComponentIdPair& idPair) override;
        void ResetValues(const AZ::EntityComponentIdPair& idPair) override;

    private:
        AZStd::unique_ptr<AzToolsFramework::BoxViewportEdit> m_boxEdit;
    };
} //namespace JoltPhysics
