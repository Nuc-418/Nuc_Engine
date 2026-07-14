# Actors and Components

NucEngine uses a UE5-style **Actor + Component** model. A `GameObject` (actor)
is a `Transform` root plus a list of `Component`s; behaviour and data live in the
components, not in the actor. This is the foundation the physics, light and
prefab features build on.

## Pieces

- **`Component`** (`src/engine/scene/Component.h`) — base class. Lifecycle hooks:
  `OnAttach`, `OnUpdate(dt)`, `OnRender(mode, camera)`, `OnUnload`, plus
  `Serialize`/`Deserialize`. Behavior hooks — `OnPlayBegin`, `OnSimulate(dt)`,
  `OnPlayEnd` — run only while the app simulates (editor Play / standalone
  game): `World::Tick` dispatches per frame and `World::NotifyPlayBegin/End`
  fire around Play/Stop. `RotatorComponent` is the reference behavior. Each
  component has a stable `TypeId()` (e.g. `"Mesh"`) used for serialization
  and the registry.
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
`World::SyncComponentLights` merges every LightComponent with the world
ambient environment term into `World::combinedLights` and re-uploads to every
lit program (`World::litPrograms`) only when something changed. Both the model
and primitive shaders read the same light uniforms, so a Light actor lights
everything — primitives included. Lights are **component-only**: the only
world-level light is the ambient term (the **Environment** panel, scene
`lights` block); directional/point/spot are always LightComponents. The demo
registers a meshless `"Light"` spawn type (and such meshless actors are
pickable in the viewport), and the Details panel edits the component.

## Cameras as components

`CameraComponent` (`src/engine/render/CameraComponent.h`) holds the lens
(fov/near/far); the pose comes from the owner's world transform. One object
can be the world's **active camera** (`World::activeCameraId`, set via the
Details panel's *Make Active Camera*, persisted in the scene file): while the
simulation runs — Play in the editor, or a standalone game build — rendering
goes through `World::ActiveCamera()`. With no active camera set, everything
renders through `World::camera` exactly as before, and the editor viewport
always uses the editor camera in Edit mode.

## Property reflection (editor UI + undo)

A component's `Serialize` doubles as its property enumeration. The Details
panel captures the fields through an `ISerializer` visitor and auto-generates
one widget per field — float/int/bool/vec3/string, plus `WriteColor` (color
picker) and `WriteEnum` (labeled combo) hints — then applies edits back
through `Deserialize`. This works for any registered component, including
plugin ones the editor cannot name (PhysicsBody, for example). Each widget
edit is one undoable ComponentEdit action: before/after snapshots in a
`FieldStore` (an in-memory ISerializer/IDeserializer, also used by tests),
re-applied via `Deserialize` on undo/redo.

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

A plugin-provided component follows the same steps from inside the plugin —
`PhysicsBodyComponent` (registered by the JoltPhysics plugin, no core change)
is the reference: add it to any object and the plugin realizes a box body from
the mesh AABB and the object's scale (static or dynamic), keeps it following
the object in Edit mode, and syncs simulated poses back while playing. The
component owns its body's lifetime (created on the plugin's next update,
removed on unload/destroy) and serializes its `dynamic` flag.
