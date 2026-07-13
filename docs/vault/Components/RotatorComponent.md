---
tags: [component, behavior]
---

# RotatorComponent

`src/engine/scene/RotatorComponent.{h,cpp}` · TypeId `"Rotator"`

The reference **behavior component** (UE's RotatingMovement equivalent):
spins its owner while the simulation runs, via `OnSimulate` — so it
animates in Play mode and game builds, never while editing.

`radiansPerSecond` is a vec3 in the engine's Euler convention (x = yaw
about Y, y = pitch about X, z = roll), applied through `Transform::Rotate`.
Serializable and editable in Details via
[[Serialization & Reflection|reflection]].

The demo's two Iron Man models spin with Rotator components.
