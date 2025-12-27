# pragma once

#include<Jolt/Jolt.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>

namespace JoltPhysics
{
    class JoltPhysicsMaterial : public JPH::PhysicsMaterial
    {
    public:
        JoltPhysicsMaterial() = default;
        JoltPhysicsMaterial(const std::string_view &inName, JPH::ColorArg inColor, float inFriction, float inRestitution, float inDensity);
        ~JoltPhysicsMaterial() override;

        void SetRestitution(float restitution);
        float GetRestitution() const;

        void SetFriction(float friction);
        float GetFriction() const;

        void SetDensity(float density);
        float GetDensity() const;

        JPH::Color GetColor() const;

        void* m_userData = nullptr;
        
    private:
        JPH::String m_debugName;
        JPH::Color m_debugColor = JPH::Color::sWhite;

        float m_restitution = 0.2f;
        float m_friction = 0.2f;
        float m_density = 1.0f;
    };
} // JoltPhysics
