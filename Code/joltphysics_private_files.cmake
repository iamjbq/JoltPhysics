
set(FILES
		Include/JoltPhysics/JoltPhysicsBus.h
		Include/JoltPhysics/JoltPhysicsTypeIds.h
		Include/JoltPhysics/JoltRigidBodyInterface.h
		Include/JoltPhysics/JoltPhysicsTypes.h
		Include/JoltPhysics/MathConversions.h
		Include/JoltPhysics/Configuration/JoltConfiguration.h
		Include/JoltPhysics/Material/JoltMaterial.h
		Include/JoltPhysics/Material/JoltMaterialConfiguration.h
		Include/JoltPhysics/BodyData.h
		Include/JoltPhysics/BodyData.inl
		Include/JoltPhysics/Utils.h
		Include/JoltPhysics/Utils.inl
		Include/JoltPhysics/ColliderComponentBus.h
		Include/JoltPhysics/ColliderShapeBus.h
		Include/JoltPhysics/NativeTypeIdentifiers.h

        Source/JoltPhysicsModuleInterface.cpp
        Source/JoltPhysicsModuleInterface.h
		Source/Utils.cpp
		Source/Utils.h
		Source/JoltSceneQueryHelpers.cpp
		Source/JoltSceneQueryHelpers.h

		Source/Clients/DefaultWorldComponent.cpp
		Source/Clients/DefaultWorldComponent.h
        Source/Clients/JoltPhysicsSystemComponent.cpp
        Source/Clients/JoltPhysicsSystemComponent.h
        Source/Clients/RigidBodyComponent.cpp
        Source/Clients/RigidBodyComponent.h
		Source/Clients/StaticRigidBodyComponent.cpp
		Source/Clients/StaticRigidBodyComponent.h
		Source/Clients/StaticRigidBody.cpp
		Source/Clients/StaticRigidBody.h
        Source/Clients/RigidBody.cpp
        Source/Clients/RigidBody.h
        Source/Clients/Shape.cpp
        Source/Clients/Shape.h
		Source/Clients/BaseColliderComponent.cpp
		Source/Clients/BaseColliderComponent.h
		Source/Clients/ShapeColliderComponent.cpp
		Source/Clients/ShapeColliderComponent.h

		Source/System/JoltAllocator.cpp
		Source/System/JoltAllocator.h
        Source/System/JoltJobSystemThreaded.cpp
        Source/System/JoltJobSystemThreaded.h
        Source/System/JoltSystem.cpp
        Source/System/JoltSystem.h
        Source/System/CollisionLayerFilters.cpp
        Source/System/CollisionLayerFilters.h

        Source/Configuration/JoltConfiguration.cpp
		Source/Configuration/JoltSettingsRegistryManager.cpp
		Source/Configuration/JoltSettingsRegistryManager.h

        Source/Scene/JoltScene.cpp
        Source/Scene/JoltScene.h
		Source/Scene/PhysicsSystemCallbacks.cpp
		Source/Scene/PhysicsSystemCallbacks.h
        Source/Scene/JoltSceneInterface.cpp
        Source/Scene/JoltSceneInterface.h

        Source/Material/JoltMaterial.cpp
        Source/Material/JoltMaterialConfiguration.cpp
		Source/Material/JoltPhysicsMaterial.cpp
		Source/Material/JoltPhysicsMaterial.h
		Source/Material/JoltMaterialManager.cpp
		Source/Material/JoltMaterialManager.h
)
