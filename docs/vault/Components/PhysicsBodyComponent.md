---
tags: [component, physics, plugin]
---

# PhysicsBodyComponent

`Plugins/JoltPhysics/PhysicsBodyComponent.{h,cpp}` · TypeId `"PhysicsBody"`

A Jolt rigid body attached to a GameObject — the proof that a
[[Plugin System|plugin]] component flows through every engine seam (editor
Add Component menu, Details editing via reflection, scene files) without
core changes.

## Behavior

- Attach it and the plugin **realizes a box body on its next update**:
  half-extents = the mesh's local AABB × the object's world scale (a 0.5
  half-extent box when meshless), positioned at the AABB's world center.
- `dynamic` (serialized): dynamic bodies write their simulated pose back
  into the owner's transform every step; static bodies are obstacles.
- **Edit mode**: the body teleports to follow its object (change-tracked,
  `PhysicsWorld::SetPose`) — what you place is what simulates on Play.
- The component owns the body's lifetime: unbind + remove on
  unload/destroy.

## v1 constraints

Bodies assume root-level objects (poses write the local transform); the
shape ignores rotation at creation time.

The demo's physics floor and falling cube are plain Cubes carrying this
component (floor `dynamic = false`).
