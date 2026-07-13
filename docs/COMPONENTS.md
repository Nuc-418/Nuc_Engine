# Actors and Components

NucEngine uses a UE5-style **Actor + Component** model. A `GameObject` (actor)
is a `Transform` root plus a list of `Component`s; behaviour and data live in the
components, not in the actor. This is the foundation the physics, light and
prefab features build on.

## Pieces

- **`Component`** (`src/engine/scene/Component.h`) — base class. Lifecycle hooks:
  `OnAttach`, `OnUpdate(dt)`, `OnRender(mode, camera)`, `OnUnload`, plus
  `Serialize`/`Deserialize`. Each component has a stable `TypeId()` (e.g.
  `"Mesh"`) used for serialization and the registry.
- **`GameObject`** (`src/engine/scene/GameObject.h`) — the actor. Owns
  `vector<unique_ptr<Component>>`. Key API:
  - `AddComponent<T>(...)`, `GetComponent<T>()` — typed access.
  - `AddComponentById(id)`, `GetComponentById(id)` — used by load / editor.
  - `Update`, `Draw`, `Unload` — dispatch to every component.
  - `GetMesh()` / `EnsureMesh()` and the `LoadObjFile` / `CreateObj*` helpers —
    mesh convenience so the spawn factories stay simple.
  - It is heap-owned by `World` and non-copyable/non-movable, so component
    back-pointers into it stay valid.
- **`MeshComponent`** (`src/engine/render/MeshComponent.h`) — the component form
  of the old `MeshRenderer` + `Material`; renders in `OnRender`.
- **`ComponentRegistry`** (`src/engine/scene/ComponentRegistry.h`) — maps a
  `TypeId` to a factory. Core registers its components; **plugins register
  theirs** (the seam that lets a plugin add a component type without core
  knowing it). Drives the editor's *Add Component* menu and scene load.

## Serialization

Components (de)serialize through engine-owned interfaces —
`ISerializer`/`IDeserializer` (`src/engine/scene/Serialization.h`) — never the
JSON library directly, so plugin components don't depend on nlohmann.
`SceneSerializer` implements these over `nlohmann::json`.

Each object writes a `components` array (`type` + state). On load, the spawn
factory first recreates the type's default components (e.g. the mesh, whose
geometry comes from the factory, not the save file); then for each saved
component the loader **reuses a matching one if present, else creates it via the
registry**, and finally lets it read its state. This reconciliation is what lets
editor-added components (physics, lights, …) round-trip while factory-owned mesh
geometry is not duplicated.

## Adding a component type

1. Subclass `Component`; give it a unique `TypeId()`.
2. Implement the hooks you need (`OnUpdate`/`OnRender`/…) and, if it has state,
   `Serialize`/`Deserialize`.
3. Register it with `ComponentRegistry::Register(id, label, factory)` (a static
   initializer in the component's `.cpp`, as `MeshComponent` does).

A plugin-provided component follows the same steps from inside the plugin, so
the physics body will be a `PhysicsBodyComponent` registered by the JoltPhysics
plugin — no core change required.
