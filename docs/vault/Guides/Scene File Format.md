---
tags: [guide, serialization, reference]
---

# Scene File Format

JSON, version 1, written by `SceneSerializer` to `assets/scenes/*.json`
(Ctrl+S / File menu; the Maps panel lists them). Loading clears the world
and respawns through the registered factories.

```json
{
  "version": 1,
  "camera": { "position": [x,y,z], "rotation": [x,y,z] },
  "renderMode": 4,
  "activeCamera": 7,                    // object id below; 0 = none
  "objects": [
    {
      "type": "Cube",                   // spawn type (may be "prefab:Name")
      "name": "Cube_3",
      "id": 12,                          // stable id, referenced by others
      "parent": 4,                       // 0 = root; resolved after load
      "position": [..], "rotation": [..], "scale": [..],   // LOCAL transform
      "components": [
        { "type": "Mesh" },
        { "type": "PhysicsBody", "dynamic": true }
      ]
    }
  ],
  "lights": { "ambient": [...], "directional": [...], "point": [...], "spot": [...] }
}
```

Key semantics:

- **Reconciliation on load**: the spawn factory creates the type's default
  components (e.g. the mesh, whose geometry is NOT in the file); each saved
  component record reuses a matching component or creates one via the
  registry, then reads its state ([[Serialization & Reflection]]).
- `parent` and `activeCamera` reference saved `id`s and resolve after all
  objects spawn — order-independent.
- The `lights` block is the world-level authored lights;
  [[LightComponent]] lights ride in each object's `components`.
- Unknown object types are skipped with a log line (e.g. a model that was
  removed from `assets/models`).

Packaged game builds load `assets/scenes/startup.json` at boot.
