
#include "JoltPhysicsMaterial.h"

namespace JoltPhysics
{
    JoltPhysicsMaterial::JoltPhysicsMaterial()
    {

    }

    JoltPhysicsMaterial::~JoltPhysicsMaterial()
    {

    }

    void JoltPhysicsMaterial::SetRestitution(float restitution)
    {
        m_restitution = restitution;
    }

    float JoltPhysicsMaterial::GetRestitution()
    {
        return m_restitution;
    }

    template<typename T>
    void JoltPhysicsMaterial::SetUserData(T* userData)
    {
        m_userData = static_cast<void*>(userData);
    }

    void* JoltPhysicsMaterial::GetUserData()
    {
        return m_userData;
    }
} // JoltPhysics
