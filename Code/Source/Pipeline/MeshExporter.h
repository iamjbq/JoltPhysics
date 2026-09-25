
#pragma once

#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>
#include <AzFramework/Physics/Material/PhysicsMaterialSlots.h>

#include <SceneAPI/SceneCore/Components/ExportingComponent.h>

namespace AZ
{
    namespace SceneAPI
    {
        namespace Events
        {
            class ExportEventContext;
        }

        namespace Containers
        {
            class Scene;
            class SceneGraph;
        }

        namespace DataTypes
        {
            class IMeshData;
        }
    }
}

namespace JoltPhysics
{
    namespace Pipeline
    {
        class MeshGroup;

        class MeshExporter
            : public AZ::SceneAPI::SceneCore::ExportingComponent
        {
        public:
            AZ_COMPONENT(MeshExporter, "{E483EEE6-6630-43F8-B724-3D6E5EE0F5BF}", AZ::SceneAPI::SceneCore::ExportingComponent);

            MeshExporter();
            ~MeshExporter() override = default;

            static void Reflect(AZ::ReflectContext* context);

            AZ::SceneAPI::Events::ProcessingResult ProcessContext(AZ::SceneAPI::Events::ExportEventContext& context) const;

        private:
            AZ::SceneAPI::Events::ProcessingResult ExportMeshObject(AZ::SceneAPI::Events::ExportEventContext& context, const AZStd::shared_ptr<const AZ::SceneAPI::DataTypes::IMeshData>& meshToExport, const AZStd::string& nodePath, const Pipeline::MeshGroup& pxMeshGroup) const;
        };

        namespace Utils
        {
            //! A struct to store the materials of the mesh nodes selected in a mesh group.
            struct AssetMaterialsData
            {
                //! Material names coming from the source scene file.
                AZStd::vector<AZStd::string> m_sourceSceneMaterialNames;

                //! Look-up table for sourceSceneMaterialNames.
                AZStd::unordered_map<AZStd::string, size_t> m_materialIndexByName;

                //! Map of mesh nodes to their list of material indices associated to each face.
                AZStd::unordered_map<AZStd::string, AZStd::vector<AZ::u16>> m_nodesToPerFaceMaterialIndices;
            };

            //! Returns the list of materials assigned to the triangles
            //! of the mesh nodes selected in a mesh group.
            AZStd::optional<AssetMaterialsData> GatherMaterialsFromMeshGroup(
                const MeshGroup& meshGroup,
                const AZ::SceneAPI::Containers::SceneGraph& sceneGraph);

            //! Function to update a list of physics material slots from a new list.
            //! All those new materials not found in the previous list will fallback to default physics material.
            void UpdateAssetPhysicsMaterials(
                const AZStd::vector<AZStd::string>& newMaterials,
                Physics::MaterialSlots& physicsMaterialSlots);
        } // namespace Utils
    } // namespace Pipeline
} // namespace JoltPhysics
