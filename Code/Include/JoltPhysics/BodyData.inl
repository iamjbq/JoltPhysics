#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>

#include <AzFramework/Physics/Common/PhysicsSimulatedBody.h>

namespace JoltPhysics
{
    inline BodyData::BodyData(JPH::Body* inBody)
    {
        auto nullUserData = [](JPH::Body* bodyToSet)
        {
            // making sure it is empty
            bodyToSet->SetUserData(0);
        };

        m_joltBody = JoltBodyUniquePtr(inBody, nullUserData);
        m_joltBody->SetUserData(reinterpret_cast<const uintptr_t>(this));
    }
    
    inline BodyData::BodyData(BodyData&& inBodyData)
        : m_sanity(inBodyData.m_sanity)
        , m_joltBody(AZStd::move(inBodyData.m_joltBody))
        , m_payload(AZStd::move(inBodyData.m_payload))
    {
        m_joltBody->SetUserData(inBodyData.m_joltBody->GetUserData());
    }

    inline BodyData& BodyData::operator=(BodyData&& inBodyData)
    {
        m_sanity = inBodyData.m_sanity;
        m_joltBody = AZStd::move(inBodyData.m_joltBody);
        m_joltBody->SetUserData(reinterpret_cast<const uintptr_t>(this));
        m_payload = AZStd::move(inBodyData.m_payload);
        return *this;
    }

    inline bool BodyData::IsValid() const
    {
        return m_sanity == SanityValue;
    }

    inline AZ::u32 BodyData::GetSanityValue() const
    {
        return m_sanity;
    }

    inline void BodyData::Invalidate()
    {
        m_joltBody = nullptr;
        m_payload = Payload();
    }

    inline AZ::EntityId BodyData::GetEntityId() const
    {
        return m_payload.m_entityId;
    }

    inline void BodyData::SetEntityId(AZ::EntityId entityId)
    {
        m_payload.m_entityId = entityId;
    }

    inline AzPhysics::SimulatedBodyHandle BodyData::GetBodyHandle() const
    {
        AzPhysics::SimulatedBody* body = GetSimulatedBody();
        if (body)
        {
            return body->m_bodyHandle;
        }
        return AzPhysics::InvalidSimulatedBodyHandle;
    }

    inline AzPhysics::RigidBody* BodyData::GetRigidBody() const
    {
        return m_payload.m_rigidBody;
    }

    inline void BodyData::SetRigidBody(AzPhysics::RigidBody* rigidBody)
    {
        m_payload.m_rigidBody = rigidBody;
    }

    inline AzPhysics::StaticRigidBody* BodyData::GetRigidBodyStatic() const
    {
        return m_payload.m_staticRigidBody;
    }

    inline void BodyData::SetRigidBodyStatic(AzPhysics::StaticRigidBody* rigidBody)
    {
        m_payload.m_staticRigidBody = rigidBody;
    }

    inline Physics::Character* BodyData::GetCharacter() const
    {
        return m_payload.m_character;
    }

    inline void BodyData::SetCharacter(Physics::Character* character)
    {
        m_payload.m_character = character;
    }

    inline Physics::RagdollNode* BodyData::GetRagdollNode() const
    {
        return m_payload.m_ragdollNode;
    }

    inline void BodyData::SetRagdollNode(Physics::RagdollNode* ragdollNode)
    {
        m_payload.m_ragdollNode = ragdollNode;
    }

    inline AzPhysics::SimulatedBody* BodyData::GetArticulationLink()
    {
        return m_payload.m_articulationLink;
    }

    inline void BodyData::SetArticulationLink(AzPhysics::SimulatedBody* articulationLink)
    {
        m_payload.m_articulationLink = articulationLink;
    }

    inline AzPhysics::SimulatedBody* BodyData::GetSimulatedBody() const
    {
        if (m_payload.m_rigidBody)
        {
            return m_payload.m_rigidBody;
        }

        else if (m_payload.m_staticRigidBody)
        {
            return m_payload.m_staticRigidBody;
        }

        else if (m_payload.m_character)
        {
            return m_payload.m_character;
        }

        else if (m_payload.m_ragdollNode)
        {
            return m_payload.m_ragdollNode;
        }

        else if (m_payload.m_articulationLink)
        {
            return m_payload.m_articulationLink;
        }

        else
        {
            AZ_Error("Jolt Body User Data", false, "Invalid user data");
            return nullptr;
        }
    }
}
