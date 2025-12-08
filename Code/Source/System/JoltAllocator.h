#pragma once

#include <Jolt/Core/TempAllocator.h>
#include <AzCore/Memory/ChildAllocatorSchema.h>
#include <AzCore/Memory/SystemAllocator.h>

namespace JoltPhysics
{
    //! System allocator to be used for all .
    AZ_CHILD_ALLOCATOR_WITH_NAME(JoltAllocator, "JoltAllocator", "{3B69EBB9-A4A3-4E3A-97D2-17B3536D6578}", AZ::SystemAllocator);

    //! Implementation of the Jolt allocator interface using Open 3D Engine allocator.
    class JoltAzAllocatorCallback
        : public JPH::TempAllocator
    {
    public:
        ~JoltAzAllocatorCallback() override = default;
        void* Allocate(JPH::uint inSize) override;
        void Free(void* inAddress, JPH::uint inSize) override;
    };
}
