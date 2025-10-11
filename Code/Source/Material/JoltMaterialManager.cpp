#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/Asset/AssetManager.h>

#include <JoltPhysics/Material/JoltMaterial.h>
#include <JoltPhysics/Material/JoltMaterialConfiguration.h>
#include <Material/JoltMaterialManager.h>

namespace JoltPhysics
{
    AZStd::shared_ptr<Physics::Material> MaterialManager::CreateDefaultMaterialInternal()
    {
        const MaterialConfiguration defaultMaterialConfiguration;

        AZ::Data::Asset<Physics::MaterialAsset> defaultMaterialAsset =
            defaultMaterialConfiguration.CreateMaterialAsset();

        return CreateMaterialInternal(
            Physics::MaterialId::CreateFromAssetId(defaultMaterialAsset.GetId()),
            defaultMaterialAsset);
    }

    AZStd::shared_ptr<Physics::Material> MaterialManager::CreateMaterialInternal(
        const Physics::MaterialId& id,
        const AZ::Data::Asset<Physics::MaterialAsset>& materialAsset)
    {
        return AZStd::shared_ptr<Physics::Material>(aznew Material(id, materialAsset));
    }
}
