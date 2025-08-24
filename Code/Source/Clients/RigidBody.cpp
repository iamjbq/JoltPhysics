#include "RigidBody.h"

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
}
