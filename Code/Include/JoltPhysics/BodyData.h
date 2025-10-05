
#pragma once

#include <AzCore/Component/EntityId.h>
#include <AzFramework/Physics/Character.h>
#include <AzFramework/Physics/Ragdoll.h>
#include <AzFramework/Physics/SimulatedBodies/RigidBody.h>
#include <AzFramework/Physics/SimulatedBodies/StaticRigidBody.h>

namespace JPH
{
    class Body;
}

namespace AzPhysics
{
    struct SimulatedBody;
}

namespace JoltPhysics
{
    /// This is a class which acts as a container for useful data that is to be stored on a Jolt Body in the
    /// mUserData variable, which is a JPH::uint64. We need to cast to and from a ptr and validate it each use.
    class BodyData
    {
    public:
        BodyData() = default;
        BodyData(BodyData&& bodyData);
        BodyData& operator=(BodyData&& bodyData);

        void Invalidate();
        AZ::EntityId GetEntityId() const;
        void SetEntityId(AZ::EntityId entityId);

        AzPhysics::SimulatedBodyHandle GetBodyHandle() const;

        AzPhysics::RigidBody* GetRigidBody() const;
        void SetRigidBody(AzPhysics::RigidBody* rigidBody);

        AzPhysics::StaticRigidBody* GetRigidBodyStatic() const;
        void SetRigidBodyStatic(AzPhysics::StaticRigidBody* rigidBody);

        Physics::Character* GetCharacter() const;
        void SetCharacter(Physics::Character* character);

        Physics::RagdollNode* GetRagdollNode() const;
        void SetRagdollNode(Physics::RagdollNode* ragdollNode);

        AzPhysics::SimulatedBody* GetArticulationLink();
        void SetArticulationLink(AzPhysics::SimulatedBody* articulationLink);

        AzPhysics::SimulatedBody* GetSimulatedBody() const;

        bool IsValid() const;

    private:
        using JoltBodyUniquePtr = AZStd::unique_ptr<JPH::Body, AZStd::function<void(JPH::Body*)> >;

        /// This is an arbitrary value used to verify the cast from JPH::uint64 mUserData on a Jolt Body to BodyData
        /// is safe. If m_sanity does not have this value, then it is not safe to use the casted pointer.
        /// Helps to debug if someone is setting userData pointer to something other than this class during development
        static constexpr AZ::u32 SanityValue = 0xf005ba11;

        AZ::u32 m_sanity = SanityValue;
        JoltBodyUniquePtr m_body;

        struct Payload
        {
            AZ::EntityId m_entityId;
            // Possible references, only one of them is not nullptr
            AzPhysics::RigidBody* m_rigidBody = nullptr;
            AzPhysics::StaticRigidBody* m_staticRigidBody = nullptr;
            Physics::Character* m_character = nullptr;
            Physics::RagdollNode* m_ragdollNode = nullptr;
            AzPhysics::SimulatedBody* m_articulationLink = nullptr;
            void* m_externalUserData = nullptr;
        };

        Payload m_payload;
    };
}

#include <JoltPhysics/BodyData.inl>
