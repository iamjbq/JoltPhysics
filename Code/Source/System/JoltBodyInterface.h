
#pragma once

#include <AzCore/Interface/Interface.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>

#include <System/BodyInterface.h>

#include "AzCore/Memory/AllocatorInstance.h"
#include "AzCore/Memory/Memory_fwd.h"
#include "AzCore/Memory/SystemAllocator.h"

namespace JoltPhysics
{
    class JoltBodyInterface final
        : public AZ::Interface<JoltPhysics::BodyInterface>::Registrar
    {
    public:
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_RTTI(JoltBodyInterface, "{F9C68DD1-5994-4A94-9135-72A7091F83B3}", JoltPhysics::BodyInterface)

         JoltBodyInterface() = default;
        ~JoltBodyInterface() override = default;

    private:
        AZStd::unique_ptr<JPH::Body> m_bodyInterface;
    };
} // JoltPhysics
