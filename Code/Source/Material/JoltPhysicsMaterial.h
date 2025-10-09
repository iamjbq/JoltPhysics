# pragma once

#include<Jolt/Jolt.h>
#include <Jolt/Physics/Collision/PhysicsMaterial.h>

namespace JoltPhysics
{
    class JoltPhysicsMaterial : public JPH::PhysicsMaterial
    {
    public:
        JoltPhysicsMaterial();
        ~JoltPhysicsMaterial();

        void SetRestitution(float restitution);
        float GetRestitution();

        template<typename T>
        void SetUserData(T* userData);
        void* GetUserData();

    private:
        void* m_userData = nullptr;

        // TODO: create JoltMaterialConfiguration to populate these values at contruction
        float m_restitution;
        float m_friction;
    };
} // JoltPhysics
