# pragma once

#include<Jolt/Jolt.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>

namespace JoltPhysics
{
    struct JoltCombineMode
    {
        enum class Enum : JPH::uint8
        {
            Average,
            Minimum,
            Maximum,
            Multiply,

            ENUM_COUNT
        };
    };
    
    class JoltPhysicsMaterial : public JPH::PhysicsMaterial
    {
    public:
        JoltPhysicsMaterial() = default;
        JoltPhysicsMaterial(const std::string_view &inName, JPH::ColorArg inColor, float inFriction, float inRestitution);
        ~JoltPhysicsMaterial();

        void SetRestitution(float restitution);
        float GetRestitution() const;

        void SetFriction(float friction);
        float GetFriction() const;

        void SetDensity(float density);
        float GetDensity() const;

        template<typename T>
        void SetUserData(T* userData);
        void* GetUserData() const;

        void SetRestitutionCombineMode(JoltCombineMode::Enum mode);
        JoltCombineMode::Enum GetRestitutionCombineMode() const;

        void SetFrictionCombineMode(JoltCombineMode::Enum mode);
        JoltCombineMode::Enum GetFrictionCombineMode() const;

    private:
        void* m_userData = nullptr;

        JPH::String m_debugName;
        JPH::Color m_debugColor = JPH::Color::sWhite;

        // TODO: create JoltMaterialConfiguration to populate these values at construction
        float m_restitution;
        float m_friction;
        float m_density;
        
        JoltCombineMode::Enum m_frictionCombineMode = JoltCombineMode::Enum::Average;
        JoltCombineMode::Enum m_restitutionCombineMode = JoltCombineMode::Enum::Maximum;
    };
} // JoltPhysics
