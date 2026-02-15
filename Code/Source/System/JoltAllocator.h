#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>

namespace JoltPhysics
{
    //! Implementation of the Jolt allocator interface using a static_buffer_allocator
    template<AZStd::size_t Size>
    class JoltAzAllocatorCallback
        : public JPH::TempAllocator
    {
        static_assert(Size > 0, "Size of static buffer must be not be 0");
        typedef JoltAzAllocatorCallback<Size> this_type;
    public:
        ~JoltAzAllocatorCallback() override = default;

        AZ_FORCE_INLINE void* Allocate(JPH::uint inSize) override
        {
            void* ptr = m_tempAllocator.allocate(inSize);
            AZ_Assert((reinterpret_cast<size_t>(ptr) & 15) == 0, "Jolt requires 16-byte aligned memory allocations.");
            return ptr;
        }

        AZ_FORCE_INLINE void Free(void* inAddress, JPH::uint inSize) override
        {
            m_tempAllocator.deallocate(inAddress, inSize);
        }

    private:
        AZStd::static_buffer_allocator<Size, 16> m_tempAllocator;
    };
}
