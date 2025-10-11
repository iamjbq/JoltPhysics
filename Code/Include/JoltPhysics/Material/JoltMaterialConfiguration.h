#pragma once

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/std/string/string.h>
#include <AzCore/Math/Color.h>

#include <JoltPhysics/Material/JoltMaterial.h>

namespace JoltPhysics
{
    //! Properties of a Jolt material.
    struct MaterialConfiguration
    {
        AZ_TYPE_INFO(JoltPhysics::MaterialConfiguration, "{B3AF6F27-BEAD-4C97-B5AE-7659A141879A}");

        static void Reflect(AZ::ReflectContext* context);

        float m_Friction = 0.5f;
        float m_restitution = 0.5f;
        float m_density = 1000.0f;

        CombineMode m_restitutionCombine = CombineMode::Average;
        CombineMode m_frictionCombine = CombineMode::Maximum;

        AZ::Color m_debugColor = AZ::Colors::White;

        //! Creates a Physics Material Asset with random Id from the
        //! properties of material configuration.
        AZ::Data::Asset<Physics::MaterialAsset> CreateMaterialAsset() const;

        static void ValidateMaterialAsset(AZ::Data::Asset<Physics::MaterialAsset> materialAsset);
        
    private:
        static float GetMinDensityLimit();
        static float GetMaxDensityLimit();        
    };
}

