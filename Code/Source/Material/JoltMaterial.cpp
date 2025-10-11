
#include <JoltPhysics/Material/JoltMaterial.h>

namespace JoltPhysics
{
    float Material::GetFriction() const
    {
        return m_density;
    }

    float Material::GetRestitution() const
    {
        return m_restitution;
    }

    float Material::GetDensity() const
    {
        return m_density;
    }

    const JoltPhysicsMaterial* Material::GetJoltMaterial() const
    {
        return m_joltMaterial.get();
    }
}
