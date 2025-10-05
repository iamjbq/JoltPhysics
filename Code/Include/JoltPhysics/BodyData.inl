
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

        m_body = JoltBodyUniquePtr(inBody, nullUserData);
        inBody->SetUserData(0); // TODO: Cast this from ptr to uint64
    }
    
    inline BodyData::BodyData(BodyData&& other)
        : m_sanity(other.m_sanity)
        , m_actor(AZStd::move(other.m_actor))
        , m_payload(AZStd::move(other.m_payload))
    {
        m_actor->userData = this;
    }

    inline BodyData& BodyData::operator=(BodyData&& other)
    {
        m_sanity = other.m_sanity;
        m_actor = AZStd::move(other.m_actor);
        m_actor->userData = this;
        m_payload = AZStd::move(other.m_payload);
        return *this;
    }

    inline bool BodyData::IsValid() const
    {
        return m_sanity == SanityValue;
    }

    inline void BodyData::Invalidate()
    {
        m_actor = nullptr;
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
