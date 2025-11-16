
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/Interface/Interface.h>
#include <AzFramework/Physics/Character.h>
#include <AzFramework/Physics/SimulatedBodies/RigidBody.h>
#include <AzFramework/Physics/Shape.h>
#include <AzFramework/Physics/ShapeConfiguration.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/CollisionBus.h>
#include <AzFramework/Physics/Configuration/CollisionConfiguration.h>
#include <AzFramework/Physics/Collision/CollisionGroups.h>
#include <AzFramework/Physics/Collision/CollisionLayers.h>
#include <AzFramework/Physics/Common/PhysicsEvents.h>

#include <JoltPhysics/JoltPhysicsBus.h>
#include <JoltPhysics/Configuration/JoltConfiguration.h>
#include <CLients/DefaultWorldComponent.h>

namespace AzPhysics
{
    struct StaticRigidBodyConfiguration;
    struct RigidBodyConfiguration;
    struct SimulatedBody;
}

namespace JoltPhysics
{
    class MaterialManager;
    class JoltSystem;

    class JoltPhysicsSystemComponent
        : public AZ::Component
        , public Physics::SystemRequestBus::Handler
        // , public JoltPhysics::SystemRequestsBus::Handler
        // , private Physics::CollisionRequestBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(JoltPhysicsSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        JoltPhysicsSystemComponent();
        ~JoltPhysicsSystemComponent();

    protected:
        // Physics::SystemRequestBus overrides...
        AZStd::shared_ptr<Physics::Shape> CreateShape(const Physics::ColliderConfiguration& colliderConfiguration, const Physics::ShapeConfiguration& configuration) override;
        void ReleaseNativeMeshObject(void* nativeMeshObject) override;
        void ReleaseNativeHeightfieldObject(void* nativeHeightfieldObject) override;
        bool CookConvexMeshToFile(const AZStd::string& filePath, const AZ::Vector3* vertices, AZ::u32 vertexCount) override;
        bool CookConvexMeshToMemory(const AZ::Vector3* vertices, AZ::u32 vertexCount, AZStd::vector<AZ::u8>& result) override;
        bool CookTriangleMeshToFile(const AZStd::string& filePath, const AZ::Vector3* vertices, AZ::u32 vertexCount,
            const AZ::u32* indices, AZ::u32 indexCount) override;
        bool CookTriangleMeshToMemory(const AZ::Vector3* vertices, AZ::u32 vertexCount,
            const AZ::u32* indices, AZ::u32 indexCount, AZStd::vector<AZ::u8>& result) override;

        // CollisionRequestBus overrides...
        // AzPhysics::CollisionLayer GetCollisionLayerByName(const AZStd::string& layerName) override;
        // AZStd::string GetCollisionLayerName(const AzPhysics::CollisionLayer& layer) override;
        // bool TryGetCollisionLayerByName(const AZStd::string& layerName, AzPhysics::CollisionLayer& layer) override;
        // AzPhysics::CollisionGroup GetCollisionGroupByName(const AZStd::string& groupName) override;
        // bool TryGetCollisionGroupByName(const AZStd::string& layerName, AzPhysics::CollisionGroup& group) override;
        // AZStd::string GetCollisionGroupName(const AzPhysics::CollisionGroup& collisionGroup) override;
        // AzPhysics::CollisionGroup GetCollisionGroupById(const AzPhysics::CollisionGroups::Id& groupId) override;
        // void SetCollisionLayerName(int index, const AZStd::string& layerName) override;
        // void CreateCollisionGroup(const AZStd::string& groupName, const AzPhysics::CollisionGroup& group) override;
        // bool ShouldCollide(
        //     const Physics::ColliderConfiguration& colliderConfigurationA,
        //     const Physics::ColliderConfiguration& colliderConfigurationB) override;

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZTickBus interface implementation
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        int GetTickOrder() override;
        ////////////////////////////////////////////////////////////////////////

    private:
        // Not sure if this is needed for Jolt implementation, but maybe a good switch to have
        void EnableAutoManagedPhysicsTick(bool shouldTick);

        void ActivateSimulation();

        bool m_enabled; ///< If false, this component will not activate itself in the Activate() function.

        AZ::Interface<Physics::CollisionRequests> m_collisionRequests;
        AZStd::unique_ptr<MaterialManager> m_materialManager;
        AZ::Interface<Physics::System> m_physicsSystem;
        DefaultWorldComponent m_defaultWorldComponent;

        JoltSystem* m_joltSystem = nullptr;
        bool m_isTickingPhysics = false;
        AzPhysics::SystemEvents::OnInitializedEvent::Handler m_onSystemInitializedHandler;
        AzPhysics::SystemEvents::OnConfigurationChangedEvent::Handler m_onSystemConfigChangedHandler;

    };

} // namespace JoltPhysics
