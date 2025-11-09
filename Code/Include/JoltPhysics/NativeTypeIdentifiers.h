#pragma once

#include <AzCore/Math/Crc.h>

namespace JoltPhysics
{
    namespace NativeTypeIdentifiers
    {
        static const AZ::Crc32 World = AZ_CRC_CE("JoltWorld");
        static const AZ::Crc32 RigidBody = AZ_CRC_CE("JoltRigidBody");
        static const AZ::Crc32 RigidBodyStatic = AZ_CRC_CE("JoltRigidBodyStatic");
        static const AZ::Crc32 ArticulationLink = AZ_CRC_CE("JoltArticulationLink");
        static const AZ::Crc32 D6Joint = AZ_CRC_CE("JoltD6Joint");
        static const AZ::Crc32 CharacterController = AZ_CRC_CE("JoltCharacterController");
        static const AZ::Crc32 CharacterControllerReplica = AZ_CRC_CE("JoltCharacterControllerReplica");
        static const AZ::Crc32 RagdollNode = AZ_CRC_CE("JoltRagdollNode");
        static const AZ::Crc32 Ragdoll = AZ_CRC_CE("JoltRagdoll");
        static const AZ::Crc32 FixedJoint = AZ_CRC_CE("JoltFixedJoint");
        static const AZ::Crc32 HingeJoint = AZ_CRC_CE("JoltHingeJoint");
        static const AZ::Crc32 BallJoint = AZ_CRC_CE("JoltBallJoint");
        static const AZ::Crc32 PrismaticJoint = AZ_CRC_CE("JoltPrismaticJoint");
    } // namespace NativeTypeIdentifiers
}
