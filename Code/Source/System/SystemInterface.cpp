
#include "SystemInterface.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Interface/Interface.h>

namespace JoltPhysics
{
    void SystemInterface::Reflect(AZ::ReflectContext* context)
    {
        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<SystemInterface>("PhysicsSystemInterface")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Script::Attributes::Module, "physics")
                ->Attribute(AZ::Script::Attributes::Category, "Physics")
                // ->Method("GetSceneHandle", &SystemInterface::GetSceneHandle)
                // ->Attribute(AZ::Script::Attributes::ExcludeFrom, AZ::Script::Attributes::ExcludeFlags::All)
                // ->Method("GetScene", &SystemInterface::GetScene)
                // ->Attribute(AZ::Script::Attributes::ExcludeFrom, AZ::Script::Attributes::ExcludeFlags::All)
                ;
        
            behaviorContext->Method(
                    "GetPhysicsSystem",
                    []()
                    {
                        return AZ::Interface<JoltPhysics::SystemInterface>::Get();
                    });
        }
    }
}
