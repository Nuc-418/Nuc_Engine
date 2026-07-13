---
tags: [architecture, scene]
---

# Components

`src/engine/scene/Component.h` — the base for everything attachable to a
[[GameObject & Hierarchy|GameObject]]. The engine core knows only this base
class, so [[Plugin System|plugins]] can define component types too.

## Identity

- `TypeId()` — stable string id ("Mesh", "Light", …), used by serialization
  and the registry. Classes also expose `static StaticTypeId()` so
  `GetComponent<T>()` can match without RTTI.
- `DisplayName()` — editor label (defaults to the TypeId).

## Lifecycle hooks

| Hook | When |
|---|---|
| `OnAttach` | Once, right after `owner` is set |
| `OnUpdate(dt)` | Every frame ([[World]]::Tick) |
| `OnRender(mode, camera)` | Draw dispatch |
| `OnUnload` | Object destroyed / world cleared (GL context current) |

## Behavior hooks

Run **only while the app simulates** (editor Play mode or standalone game)
— gameplay written here never runs while editing:

| Hook | When |
|---|---|
| `OnPlayBegin` | Simulation starts (Play pressed / game boot) |
| `OnSimulate(dt)` | Every simulating frame |
| `OnPlayEnd` | Simulation stops (Stop pressed) |

[[RotatorComponent]] is the reference behavior.

## Serialization

`Serialize(ISerializer&)` / `Deserialize(const IDeserializer&)` — see
[[Serialization & Reflection]]. The same `Serialize` drives the editor's
auto-generated Details UI and the [[Undo System|component-edit undo]].

## The ComponentRegistry

`ComponentRegistry::Register(typeId, label, factory)` maps string ids to
factories. It drives the editor's *Add Component* menu and scene-load
reconstruction. Engine components register via static initializers in their
`.cpp`; plugin components register in `EnginePlugin::RegisterTypes()`
(safe against static-lib dead-stripping).

## Built-in component types

[[MeshComponent]] · [[LightComponent]] · [[CameraComponent]] ·
[[RotatorComponent]] · [[PhysicsBodyComponent]] (plugin)
