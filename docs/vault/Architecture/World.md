---
tags: [architecture, scene]
---

# World

`src/engine/scene/World.{h,cpp}` — the registry of scene objects plus the
lights, cameras and render settings that belong to the running scene.

## Object registry

- `entries` — flat owning store: each `WorldEntry` has a `typeId` (the spawn
  type), a **stable numeric `id`** (drives the [[Undo System]]) and a
  `unique_ptr<GameObject>`. Objects are never copied or moved, so pointers
  into them stay valid.
- `RegisterType(id, label, factory)` — registers a spawnable type by string
  id. The demo registers primitives, discovered models, `Light`, `Camera`;
  [[Prefabs]] register themselves as `prefab:<name>`.
- `Spawn(id, name, forcedId)` — runs the factory, assigns a unique name and
  id. `forcedId` re-attaches a previous identity (undo of a delete).
- `Create(id)` — factory run without adding to the world (thumbnails).
- `Destroy(object)` — reparents the object's children to its parent (world
  pose kept), fires `onDestroyed`, calls `Unload`, erases the entry.

## Scene state

- `lights` — world-level authored lights; `combinedLights` — those merged
  with every [[LightComponent]] (this is what is actually uploaded).
  `SyncComponentLights()` re-uploads only when something changed.
- `camera` — the editor/default viewport camera. `activeCameraId` selects a
  [[CameraComponent]] object to render the running game through
  (`ActiveCamera()`), falling back to `camera`.
- `renderMode` — GL primitive mode (demo debug feature, keys 6–9).

## Dispatch

- `Tick(dt, simulating)` — `OnUpdate` for every component, plus `OnSimulate`
  while simulating (see [[Components#Behavior hooks]]).
- `NotifyPlayBegin()/NotifyPlayEnd()` — fired around editor Play/Stop and
  game boot.
