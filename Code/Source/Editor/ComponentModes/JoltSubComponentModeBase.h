
#pragma once

namespace AZ
{
    class EntityComponentIdPair;
} // namespace AZ

namespace AzToolsFramework::ViewportInteraction
{
    struct MouseInteractionEvent;
} // namespace AzToolsFramework::ViewportInteraction

namespace JoltPhysics
{
    class JoltSubComponentModeBase
    {
    public:
        virtual ~JoltSubComponentModeBase() = default;

        //! Called when the mode is entered to initialize the mode.
        //! @param idPair The entity/component id pair.
        virtual void Setup(const AZ::EntityComponentIdPair& idPair) = 0;

        //! Called when the mode needs to refresh it's values.
        //! @param idPair The entity/component id pair.
        virtual void Refresh(const AZ::EntityComponentIdPair& idPair) = 0;

        //! Called when the mode exits to perform cleanup.
        //! @param idPair The entity/component id pair.
        virtual void Teardown(const AZ::EntityComponentIdPair& idPair) = 0;

        //! Called when reset hot key is pressed.
        //! Should reset values in the sub component mode to sensible defaults.
        //! @param idPair The entity/component id pair.
        virtual void ResetValues(const AZ::EntityComponentIdPair& idPair) = 0;

        //! Additional mouse handling by sub-component mode. Does not absorb mouse event.
        virtual void HandleMouseInteraction(
            [[maybe_unused]] const AzToolsFramework::ViewportInteraction::MouseInteractionEvent& mouseInteraction) {};
    };

    //! NullObject implementation.
    //! Use it as a placeholder for unimplemented component modes
    class NullColliderComponentMode : public JoltSubComponentModeBase
    {
    public:
        void Setup([[maybe_unused]] const AZ::EntityComponentIdPair& idPair) override {}
        void Refresh([[maybe_unused]] const AZ::EntityComponentIdPair& idPair) override{}
        void Teardown([[maybe_unused]] const AZ::EntityComponentIdPair& idPair) override{}
        void ResetValues([[maybe_unused]] const AZ::EntityComponentIdPair& idPair) override{}
    };

} // namespace JoltPhysics
