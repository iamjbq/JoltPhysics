#include <Editor/CollisionLayerWidget.h>
#include <Editor/CollisionGroupWidget.h>
// #include <Editor/InertiaPropertyHandler.h>
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>

namespace JoltPhysics
{
    namespace Editor
    {
        void RegisterPropertyTypes()
        {
            AzToolsFramework::PropertyTypeRegistrationMessages::Bus::Broadcast(&AzToolsFramework::PropertyTypeRegistrationMessages::RegisterPropertyType, aznew JoltPhysics::Editor::CollisionLayerWidget());
            AzToolsFramework::PropertyTypeRegistrationMessages::Bus::Broadcast(&AzToolsFramework::PropertyTypeRegistrationMessages::RegisterPropertyType, aznew JoltPhysics::Editor::CollisionGroupWidget());
            AzToolsFramework::PropertyTypeRegistrationMessages::Bus::Broadcast(&AzToolsFramework::PropertyTypeRegistrationMessages::RegisterPropertyType, aznew JoltPhysics::Editor::CollisionGroupEnumPropertyComboBoxHandler());
            // AzToolsFramework::PropertyTypeRegistrationMessages::Bus::Broadcast(&AzToolsFramework::PropertyTypeRegistrationMessages::RegisterPropertyType, aznew JoltPhysics::Editor::InertiaPropertyHandler());
        }

        void UnregisterPropertyTypes()
        {
            AzToolsFramework::PropertyTypeRegistrationMessages::Bus::Broadcast(&AzToolsFramework::PropertyTypeRegistrationMessages::UnregisterPropertyType, aznew JoltPhysics::Editor::CollisionLayerWidget());
            AzToolsFramework::PropertyTypeRegistrationMessages::Bus::Broadcast(&AzToolsFramework::PropertyTypeRegistrationMessages::UnregisterPropertyType, aznew JoltPhysics::Editor::CollisionGroupWidget());
            AzToolsFramework::PropertyTypeRegistrationMessages::Bus::Broadcast(&AzToolsFramework::PropertyTypeRegistrationMessages::UnregisterPropertyType, aznew JoltPhysics::Editor::CollisionGroupEnumPropertyComboBoxHandler());
            // AzToolsFramework::PropertyTypeRegistrationMessages::Bus::Broadcast(&AzToolsFramework::PropertyTypeRegistrationMessages::UnregisterPropertyType, aznew JoltPhysics::Editor::InertiaPropertyHandler());
        }
    }
}
