#include <Clients/RigidBody.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <AzCore/std/utility/as_const.h>
#include <AzCore/Math/MathStringConversions.h>
#include <AzFramework/Physics/Utils.h>
#include <AzFramework/Physics/Configuration/RigidBodyConfiguration.h>

#include <JoltPhysics/MathConversions.h>
#include <JoltPhysics/Utils.h>
#include <Clients/Shape.h>
#include <Material/JoltPhysicsMaterial.h>

namespace JoltPhysics
{
    void RigidBodyConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<JoltPhysics::RigidBodyConfiguration>()
                ->Version(1)
                // ->Field("SolverPositionIterations", &JoltPhysics::RigidBodyConfiguration::m_solverPositionIterations)
                // ->Field("SolverVelocityIterations", &JoltPhysics::RigidBodyConfiguration::m_solverVelocityIterations)
                ;

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<JoltPhysics::RigidBodyConfiguration>("JoltPhysics-specific Rigid Body Configuration",
                    "Additional Rigid Body settings specific to JoltPhysics.")
                    // ->DataElement(
                    //     AZ::Edit::UIHandlers::Default,
                    //     &JoltPhysics::RigidBodyConfiguration::m_solverPositionIterations,
                    //     "Solver Position Iterations",
                    //     "Higher values can improve stability at the cost of performance.")
                    // ->Attribute(AZ::Edit::Attributes::Min, 1)
                    // ->Attribute(AZ::Edit::Attributes::Max, 255)
                    // ->DataElement(
                    //     AZ::Edit::UIHandlers::Default,
                    //     &JoltPhysics::RigidBodyConfiguration::m_solverVelocityIterations,
                    //     "Solver Velocity Iterations",
                    //     "Higher values can improve stability at the cost of performance.")
                    // ->Attribute(AZ::Edit::Attributes::Min, 1)
                    // ->Attribute(AZ::Edit::Attributes::Max, 255)
                    ;
            }
        }
    }

    AZ::u32 RigidBody::GetShapeCount() const
    {
        return static_cast<AZ::u32>(m_shapes.size());
    }

    AZStd::shared_ptr<Physics::Shape> RigidBody::GetShape(AZ::u32 index)
    {
        AZStd::shared_ptr<const Physics::Shape> constShape = AZStd::as_const(*this).GetShape(index);
        return AZStd::const_pointer_cast<Physics::Shape>(constShape);
    }

    AZStd::shared_ptr<const Physics::Shape> RigidBody::GetShape(AZ::u32 index) const
    {
        if (index >= m_shapes.size())
        {
            return nullptr;
        }
        return m_shapes[index];
    }

    void RigidBody::UpdateMassProperties(
        AzPhysics::MassComputeFlags flags,
        const AZ::Vector3& centerOfMassOffsetOverride,
        const AZ::Matrix3x3& inertiaTensorOverride,
        const float massOverride)
    {
        const bool computeCenterOfMass = AzPhysics::MassComputeFlags::COMPUTE_COM == (flags & AzPhysics::MassComputeFlags::COMPUTE_COM);
        const bool computeInertiaTensor = AzPhysics::MassComputeFlags::COMPUTE_INERTIA == (flags & AzPhysics::MassComputeFlags::COMPUTE_INERTIA);
        const bool computeMass = AzPhysics::MassComputeFlags::COMPUTE_MASS == (flags & AzPhysics::MassComputeFlags::COMPUTE_MASS);
        const bool needsCompute = computeCenterOfMass || computeInertiaTensor || computeMass;
        const bool includeAllShapesInMassCalculation = AzPhysics::MassComputeFlags::INCLUDE_ALL_SHAPES == (flags & AzPhysics::MassComputeFlags::INCLUDE_ALL_SHAPES);
    }
}
