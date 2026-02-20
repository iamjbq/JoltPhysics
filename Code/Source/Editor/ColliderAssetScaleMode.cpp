
#include <Editor/ColliderAssetScaleMode.h>
#include <JoltPhysics/EditorColliderComponentRequestBus.h>

#include <AzToolsFramework/Manipulators/ManipulatorManager.h>
#include <AzCore/Component/TransformBus.h>
#include <AzFramework/Viewport/ViewportColors.h>
#include <AzFramework/Viewport/ViewportConstants.h>
#include <AzToolsFramework/Manipulators/AngularManipulator.h>
#include <AzToolsFramework/ViewportSelection/EditorSelectionUtil.h>

namespace JoltPhysics
{
    AZ_CLASS_ALLOCATOR_IMPL(ColliderAssetScaleMode, AZ::SystemAllocator);

    namespace
    {
        const float MinAssetScale = 0.001f;
        const AZ::Vector3 ResetScale = AZ::Vector3::CreateOne();
    }

    ColliderAssetScaleMode::ColliderAssetScaleMode()
        : m_dimensionsManipulators(AZ::Transform::Identity())
    {

    }

    void ColliderAssetScaleMode::Setup(const AZ::EntityComponentIdPair& idPair)
    {
        AZ::Transform colliderWorldTransform = AZ::Transform::Identity();
        JoltPhysics::EditorColliderComponentRequestBus::EventResult(colliderWorldTransform, idPair, &JoltPhysics::EditorColliderComponentRequests::GetColliderWorldTransform);

        m_dimensionsManipulators.SetSpace(colliderWorldTransform);
        m_dimensionsManipulators.AddEntityComponentIdPair(idPair);
        m_dimensionsManipulators.Register(AzToolsFramework::GetMainManipulatorManagerId());
        m_dimensionsManipulators.SetAxes(
            AZ::Vector3::CreateAxisX(),
            AZ::Vector3::CreateAxisY(),
            AZ::Vector3::CreateAxisZ());
        m_dimensionsManipulators.ConfigureView(
            AzFramework::ViewportConstants::DefaultLinearManipulatorAxisLength,
            AzFramework::ViewportColors::XAxisColor,
            AzFramework::ViewportColors::YAxisColor,
            AzFramework::ViewportColors::ZAxisColor);

        m_dimensionsManipulators.InstallAxisLeftMouseDownCallback(
            [this, idPair] (const AzToolsFramework::LinearManipulator::Action& action)
        {
            AZ_UNUSED(action);
            OnManipulatorDown(idPair);
        });

        m_dimensionsManipulators.InstallAxisMouseMoveCallback(
            [this, idPair] (const AzToolsFramework::LinearManipulator::Action& action)
        {
            OnManipulatorMoved(action.m_start.m_sign * action.LocalScaleOffset() + m_initialScale, idPair);
        });

        m_dimensionsManipulators.InstallUniformLeftMouseDownCallback(
            [this, idPair](const AzToolsFramework::LinearManipulator::Action& action)
        {
            AZ_UNUSED(action);
            OnManipulatorDown(idPair);
        });

        m_dimensionsManipulators.InstallUniformMouseMoveCallback(
            [this, idPair] (const AzToolsFramework::LinearManipulator::Action& action)
        {
            // Uniform scale manipulator will only set the Z value for the offset, use this to create uniform scale.
            const AZ::Vector3 uniformScale(action.LocalScaleOffset().GetZ());
            OnManipulatorMoved(uniformScale + m_initialScale, idPair);
        });
    }

    void ColliderAssetScaleMode::Refresh(const AZ::EntityComponentIdPair& idPair)
    {
        AZ::Transform colliderWorldTransform = AZ::Transform::Identity();
        JoltPhysics::EditorColliderComponentRequestBus::EventResult(colliderWorldTransform, idPair, &JoltPhysics::EditorColliderComponentRequests::GetColliderWorldTransform);
        m_dimensionsManipulators.SetSpace(colliderWorldTransform);
    }

    void ColliderAssetScaleMode::Teardown(const AZ::EntityComponentIdPair& idPair)
    {
        m_dimensionsManipulators.RemoveEntityComponentIdPair(idPair);
        m_dimensionsManipulators.Unregister();
    }

    void ColliderAssetScaleMode::OnManipulatorDown(const AZ::EntityComponentIdPair& idPair)
    {
        // Remember the initial asset scale here.
        JoltPhysics::EditorMeshColliderComponentRequestBus::EventResult(m_initialScale, idPair, &JoltPhysics::EditorMeshColliderComponentRequests::GetAssetScale);
    }

    void ColliderAssetScaleMode::OnManipulatorMoved(const AZ::Vector3& scale, const AZ::EntityComponentIdPair& idPair)
    {
        AZ::Vector3 clampedScale = scale.GetMax(AZ::Vector3(MinAssetScale));
        JoltPhysics::EditorMeshColliderComponentRequestBus::Event(idPair, &JoltPhysics::EditorMeshColliderComponentRequests::SetAssetScale, clampedScale);
    }

    void ColliderAssetScaleMode::ResetValues(const AZ::EntityComponentIdPair& idPair)
    {
        JoltPhysics::EditorMeshColliderComponentRequestBus::Event(idPair, &JoltPhysics::EditorMeshColliderComponentRequests::SetAssetScale, ResetScale);
    }
} // namespace JoltPhysics
