---
tags: [editor, assets]
---

# Prefabs

`src/engine/io/PrefabLibrary.{h,cpp}` — GameObjects as reusable assets.

## What a prefab is

A JSON file, `assets/prefabs/<name>.prefab.json`:

```json
{
  "version": 1,
  "type": "Cube",              // base spawn type (provides geometry/shader)
  "rotation": [0, 0, 0],
  "scale": [1, 1, 1],
  "components": [ { "type": "PhysicsBody", "dynamic": true }, ... ]
}
```

Position is **not** stored — it comes from placement, like any spawn.
Prefabs may be based on other prefabs (the base type can be `prefab:<x>`).

## Workflow

1. Build an object in the editor (components, tuning).
2. Outliner right-click → **Save as Prefab** (named after the object).
3. It immediately registers as a spawn type `prefab:<name>` — appearing in
   **+ Add**, the Content Browser (thumbnail regenerates), drag-drop spawn,
   [[Undo System|undo]] respawn and [[Scene File Format|scene files]], with
   zero extra plumbing.

Spawning reconciles components through the same path as scene load
(`ReadComponentsInto` — see [[Serialization & Reflection]]).
`Prefabs::RegisterAll(world)` scans the directory at scene load, after the
base types are registered.
