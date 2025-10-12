
namespace JoltPhysics
{
    inline BodyData* Utils::GetUserData(const JPH::Body& body)
    {
        if (body.GetUserData() == 0)
        {
            return nullptr;
        }

        auto bodyData = reinterpret_cast<BodyData*>(body.GetUserData());
        if (!bodyData || !bodyData->IsValid())
        {
            AZ_Warning("JoltPhysics::Utils::GetUserData", false, "The body data does not look valid and is not safe to use");
            return nullptr;
        }

        return bodyData;
    }

    inline Physics::Material* Utils::GetUserData(const JoltPhysics::JoltPhysicsMaterial* material)
    {
        return (material == nullptr) ? nullptr : static_cast<Physics::Material*>(material->m_userData);
    }

    inline Physics::Shape* Utils::GetUserData(const JPH::Shape* shape)
    {
        return (shape == nullptr) ? nullptr : reinterpret_cast<Physics::Shape*>(shape->GetUserData());
    }
}
