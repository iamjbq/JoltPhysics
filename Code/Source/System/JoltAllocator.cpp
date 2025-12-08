
#include <System/JoltAllocator.h>

namespace JoltPhysics
{
    void* JoltAzAllocatorCallback::Allocate(JPH::uint inSize)
    {
        void* ptr = AZ::AllocatorInstance<JoltAllocator>::Get().allocate(inSize, 16);
        AZ_Assert((reinterpret_cast<size_t>(ptr) & 15) == 0, "Jolt requires 16-byte aligned memory allocations.");
        return ptr;
    }

    void JoltAzAllocatorCallback::Free(void* inAddress, JPH::uint inSize)
    {
        AZ::AllocatorInstance<JoltAllocator>::Get().deallocate(inAddress, inSize);
    }
} // JoltPhysics