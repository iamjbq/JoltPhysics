# JoltPhysics for O3DE

This repo is a standalone gem which adds Jolt as a physics backend to the Open 3D Engine. It uses O3DE AzPhysics interface and targets comparable functionality with the existing PhysX gems. The goal is to maintain API compatibility between PhysX and Jolt, with most of the implementation detail being hidden from the user.

### Current project status:
- [x] Collision layer and group filtering
- [x] Basic shape colliders
- [x] Physics materials
- [x] Static rigid bodies
- [x] Dynamic/kinematic rigid bodies
- [ ] Character controllers (basic, and advanced)
- [ ] Heightfield collider - partially complete
- [ ] ConvexHull collider
- [ ] Mesh collider
- [ ] Other advanced colliders (static and mutable compound colliders)
- [ ] Soft bodies
- [ ] Joints
- [ ] Vehicle systems (motors, wheels, controller)
- [ ] Water
