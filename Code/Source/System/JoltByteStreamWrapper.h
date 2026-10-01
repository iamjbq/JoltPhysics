#include <AzCore/IO/ByteContainerStream.h>
#include <AzCore/std/containers/vector.h>

#include <Jolt/Jolt.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>

namespace JoltPhysics
{
    class JoltByteStreamOut : public JPH::StreamOut
    {
    public:
        explicit JoltByteStreamOut(AZStd::vector<AZ::u8>& buffer)
            : m_stream(&buffer) {}

        void WriteBytes(const void* inData, size_t inNumBytes) override
        {
            if (m_failed) { return; }
            AZ::IO::SizeType written = m_stream.Write(inNumBytes, inData);
            if (written != inNumBytes)
            {
                m_failed = true;
            }
        }

        bool IsFailed() const override { return m_failed; }

    private:
        AZ::IO::ByteContainerStream<AZStd::vector<AZ::u8>> m_stream;
        bool m_failed = false;
    };

    class JoltByteStreamIn : public JPH::StreamIn
    {
    public:
        explicit JoltByteStreamIn(AZStd::vector<AZ::u8>& buffer)
            : m_stream(&buffer) {}

        void ReadBytes(void* outData, size_t inNumBytes) override
        {
            if (m_failed) { return; }
            AZ::IO::SizeType readCount = m_stream.Read(inNumBytes, outData);
            if (readCount != inNumBytes)
            {
                m_failed = true;
            }
        }

        bool IsEOF() const override { return m_stream.GetCurPos() >= m_stream.GetLength(); }
        bool IsFailed() const override { return m_failed; }

    private:
        AZ::IO::ByteContainerStream<AZStd::vector<AZ::u8>> m_stream;
        bool m_failed = false;
    };
}
