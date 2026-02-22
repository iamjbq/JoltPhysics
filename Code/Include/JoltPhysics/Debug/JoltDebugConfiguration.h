
#pragma once

#include <AzCore/Math/Color.h>
#include <AzCore/std/string/string.h>

namespace AZ
{
    class ReflectContext;
}

namespace JoltPhysics
{
    namespace Debug
    {
        struct ColliderProximityVisualization
        {
            AZ_TYPE_INFO(ColliderProximityVisualization, "{9BB7A559-E32E-48C3-8285-5010D635BF9A}");
            static void Reflect(AZ::ReflectContext* context);

            ColliderProximityVisualization() = default;
            ColliderProximityVisualization(bool enabled, const AZ::Vector3 cameraPosition, float radius)
                : m_enabled(enabled)
                , m_cameraPosition(cameraPosition)
                , m_radius(radius)
            {

            }
            bool m_enabled = false; //!< If camera proximity based collider visualization is currently active.
            AZ::Vector3 m_cameraPosition = AZ::Vector3::CreateZero(); //!< Camera position to perform proximity based collider visualization around.
            float m_radius = 1.0f; //!< The radius to visualize colliders around the camera position.

            bool operator==(const ColliderProximityVisualization& other) const;
            bool operator!=(const ColliderProximityVisualization& other) const;
        };

        //! System configuration settings for in-editor debug display of colliders and joints
        struct DebugDisplayData
        {
            AZ_TYPE_INFO(DebugDisplayData, "{0FD970BB-A4CB-4CBF-92EF-70C0EF7B66E8}");
            static void Reflect(AZ::ReflectContext* context);

            //! Center of Mass Debug Draw Circle Size
            float m_centerOfMassDebugSize = 0.1f;

            //! Center of Mass Debug Draw Circle Color
            AZ::Color m_centerOfMassDebugColor = AZ::Color((AZ::u8)255, (AZ::u8)0, (AZ::u8)0, (AZ::u8)255);

            //! Enable Global Collision Debug Draw
            enum class GlobalCollisionDebugState
            {
                AlwaysOn,         //!< Collision draw debug all entities.
                AlwaysOff,        //!< Collision debug draw disabled.
                Manual            //!< Set up in the entity.
            };
            GlobalCollisionDebugState m_globalCollisionDebugDraw = GlobalCollisionDebugState::Manual;

            //! Color scheme for debug collision
            enum class GlobalCollisionDebugColorMode
            {
                MaterialColor,   //!< Use debug color specified in the material
                ErrorColor       //!< Show default color and flashing red for colliders with errors.
            };
            GlobalCollisionDebugColorMode m_globalCollisionDebugDrawColorMode = GlobalCollisionDebugColorMode::MaterialColor;


            //! Colors for joint lead
            enum class JointLeadColor
            {
                Aquamarine,
                AliceBlue,
                CadetBlue,
                Coral,
                Green,
                DarkGreen,
                ForestGreen,
                Honeydew
            };

            //! Colors for joint follower
            enum class JointFollowerColor
            {
                Yellow,
                Chocolate,
                HotPink,
                Lavender,
                Magenta,
                LightYellow,
                Maroon,
                Red
            };

            AZ::Color GetJointLeadColor() const;
            AZ::Color GetJointFollowerColor() const;

            bool m_showJointHierarchy = true; //!< Flag to switch on/off the display of joints' lead-follower connections in the viewport.
            JointLeadColor m_jointHierarchyLeadColor = JointLeadColor::Aquamarine; //!< Color of the lead half of a lead-follower joint connection line.
            JointFollowerColor m_jointHierarchyFollowerColor = JointFollowerColor::Magenta; //!< Color of the follower half of a lead-follower joint connection line.
            float m_jointHierarchyDistanceThreshold = 1.0f; //!< Minimum distance required to draw from follower to joint. Distances shorter than this threshold will result in the line drawn from the joint to the lead.

            ColliderProximityVisualization m_colliderProximityVisualization;

            bool operator==(const DebugDisplayData& other) const;
            bool operator!=(const DebugDisplayData& other) const;
        };

        struct DebugConfiguration
        {
            AZ_TYPE_INFO(DebugConfiguration, "{735E8A84-0AC6-41E4-B806-169CC297E3D1}");
            static void Reflect(AZ::ReflectContext* context);

            static DebugConfiguration CreateDefault() { return DebugConfiguration(); }

            DebugDisplayData m_debugDisplayData;
            // PvdConfiguration m_pvdConfigurationData;

            bool operator==(const DebugConfiguration& other) const;
            bool operator!=(const DebugConfiguration& other) const;
        };
    } // namespace Debug
}
