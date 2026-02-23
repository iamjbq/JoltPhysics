
#pragma once

#include <AzToolsFramework/Manipulators/TranslationManipulators.h>
#include <Editor/ComponentModes/JoltSubComponentModeBase.h>

namespace JoltPhysics
{
    /// Sub component mode for modifying offset on a collider in the viewport.
    class ColliderOffsetMode : public JoltSubComponentModeBase
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL

        ColliderOffsetMode();

        // JoltSubComponentModeBase ...
        void Setup(const AZ::EntityComponentIdPair& idPair) override;
        void Refresh(const AZ::EntityComponentIdPair& idPair) override;
        void Teardown(const AZ::EntityComponentIdPair& idPair) override;
        void ResetValues(const AZ::EntityComponentIdPair& idPair) override;

    private:
        void OnManipulatorMoved(
            const AZ::Vector3& startPosition, const AZ::Vector3& offset, const AZ::EntityComponentIdPair& idPair);

        AzToolsFramework::TranslationManipulators m_translationManipulators;
    };
} //namespace JoltPhysics
