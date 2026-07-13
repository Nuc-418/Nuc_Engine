---
tags: [architecture, serialization]
---

# Serialization & Reflection

Components (de)serialize through **engine-owned interfaces** — never a JSON
library directly — so plugin components don't depend on nlohmann.

## The interfaces (`engine/scene/Serialization.h`)

- `ISerializer` — `Write(key, value)` for float/int/bool/vec3/string, plus
  two **presentation hints** that default to the plain write:
  - `WriteColor(key, vec3)` — draw a color picker in the editor
  - `WriteEnum(key, int, labels, count)` — draw a labeled combo
- `IDeserializer` — `Read*(key, fallback)`.

## Implementations

| Implementation | Where | Used for |
|---|---|---|
| `JsonWriter` / `JsonReader` | `engine/io/JsonSerialization.h` | [[Scene File Format|scene files]], [[Prefabs]] |
| `FieldStore` | `engine/scene/FieldStore.h` | in-memory snapshots: [[Undo System|component undo]], unit tests |
| `PropertyCapture` / `PropertyApply` | `DetailsPanel.cpp` | the reflection UI below |

## Property reflection

A component's `Serialize` **doubles as its property enumeration**. The
Details panel captures the fields through `PropertyCapture`, draws one
widget per field (drags, checkbox, color picker, enum combo, text), and
applies edits back through `Deserialize`. This works for *any* registered
component — including plugin types the editor cannot name, like
[[PhysicsBodyComponent]]. Each widget edit becomes one undoable
`ComponentEdit` action (before/after `FieldStore` snapshots).

## Shared JSON helpers (`engine/io/JsonSerialization.h`)

- `ComponentsToJson(object)` — every component as `[{type, ...state}]`
- `ReadComponentsInto(object, array)` — the **reconciliation path**: reuse a
  matching component if the spawn factory already created it (e.g. the
  mesh), else create via the registry, then `Deserialize`. Scene load and
  prefab spawn share this exact logic.
