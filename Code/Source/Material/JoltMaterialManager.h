#pragma once

#include <AzFramework/Physics/Material/PhysicsMaterialManager.h>

namespace JoltPhysics
{
    //! Material manager specialization for Jolt.
    class MaterialManager
        : public AZ::Interface<Physics::MaterialManager>::Registrar
    {
    public:
        AZ_RTTI(JoltPhysics::MaterialManager, "{7268BF48-75A9-4629-81CC-F5D89AF4B422}", Physics::MaterialManager)

        MaterialManager() = default;

    protected:
        AZStd::shared_ptr<Physics::Material> CreateDefaultMaterialInternal() override;
        AZStd::shared_ptr<Physics::Material> CreateMaterialInternal(const Physics::MaterialId& id, const AZ::Data::Asset<Physics::MaterialAsset>& materialAsset) override;
        
    };
}

