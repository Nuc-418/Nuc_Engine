---
tags: [component, rendering]
---

# CameraComponent

`src/engine/render/CameraComponent.{h,cpp}` · TypeId `"Camera"`

A scene camera: the component holds only the lens (`fovDegrees`,
`nearPlane`, `farPlane`); the pose comes from the owner's world transform —
aim it like any actor, parent it to anything.

One object can be the world's **active camera**
(`World::activeCameraId`, set via *Make Active Camera* in the Details
panel, persisted in the [[Scene File Format|scene file]]). While the
simulation runs — editor Play or a standalone game — rendering goes through
`World::ActiveCamera()`. With none set, everything renders through
`World::camera` exactly as before; the editor viewport always uses the
editor camera in Edit mode. Destroying the object clears the active id.

The demo registers a meshless `"Camera"` spawn type.
