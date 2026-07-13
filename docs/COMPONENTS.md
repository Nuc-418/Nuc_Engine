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

## Hierarchy

`GameObject`s form a parent/child tree (UE5-style attachment). The object's
`Transform` is **local to its parent**; the world matrix composes up the chain
(`GameObject::WorldMatrix()`). Key API: `SetParent(parent, keepWorldTransform)`
(nullptr = root; cycles are refused; with `keepWorldTransform` the local
transform is rebased through the parent's inverse so the object doesn't move),
`Parent()`, `Children()`, `IsAncestorOf()`. Ownership stays flat in
`World::entries` — links are non-owning. Destroying a parent reparents its
children to the destroyed object's parent, keeping their world pose. The
editor's Outliner shows the tree and reparents by drag-and-drop (undoable);
the scene file stores each object's `id` and `parent` id.

Note: rebasing decomposes the local matrix into position/Euler/scale, so a
non-uniform-scaled, rotated parent can introduce shear the TRS form cannot
represent — standard TRS-engine behavior.

## Lights as components

`LightComponent` (`src/engine/render/LightComponent.h`) turns an actor into a
light source: kind (directional/point/spot), colors, attenuation and cone
angle live on the component; **placement comes from the owner's transform**
(world position for point/spot, world +Z forward for aim). Each frame
`World::SyncComponentLights` merges every LightComponent after the world-level
authored lights into `World::combinedLights` and re-uploads through the
existing `Lights` uniform path only when something changed — shaders are
untouched. The demo registers a meshless `"Light"` spawn type, and the Details
panel edits the component's parameters (component edits are not yet undoable).
World-level authored lights (Lights panel, scene `lights` block) continue to
work unchanged; migrating them fully onto components is a later step.

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
