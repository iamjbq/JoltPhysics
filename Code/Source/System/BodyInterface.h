
#pragma once

#include <AzCore/RTTI/RTTI.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/functional.h>

namespace AZ
{
    class ReflectContext;
}

namespace JoltPhysics
{
    class BodyInterface
    {
        public:
        AZ_RTTI(BodyInterface, "{020D139D-4374-4E74-940E-329466B2CA75}")

        BodyInterface() = default;
        virtual ~BodyInterface() = default;
        AZ_DISABLE_COPY_MOVE(BodyInterface)

        static void Reflect(AZ::ReflectContext* context);
    };
} // JoltPhysics
