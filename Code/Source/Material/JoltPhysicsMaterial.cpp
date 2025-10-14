
#include "JoltPhysicsMaterial.h"

namespace JoltPhysics
{
     JoltPhysicsMaterial::JoltPhysicsMaterial(const std::string_view& inName, JPH::ColorArg inColor, float inFriction, float inRestitution, float inDensity)
         : m_debugName(inName), m_debugColor(inColor), m_friction(inFriction), m_restitution(inRestitution), m_density(inDensity)
    {
    }

    JoltPhysicsMaterial::~JoltPhysicsMaterial()
    {

    }

    void JoltPhysicsMaterial::SetRestitution(const float restitution)
    {
        m_restitution = restitution;
    }

    float JoltPhysicsMaterial::GetRestitution() const
    {
        return m_restitution;
    }

    void JoltPhysicsMaterial::SetFriction(const float friction)
    {
        m_friction = friction;
    }

    float JoltPhysicsMaterial::GetFriction() const
    {
        return m_friction;
    }

    void JoltPhysicsMaterial::SetDensity(float density)
    {
        m_density = density;
    }

    float JoltPhysicsMaterial::GetDensity() const
    {
        return m_density;
    }
} // JoltPhysics
