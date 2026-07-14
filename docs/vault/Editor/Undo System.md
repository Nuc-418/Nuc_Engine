---
tags: [editor]
---

# Undo System

`src/engine/editor/UndoStack.{h,cpp}` — command history keyed by objects'
**stable World ids**, so history survives delete/respawn cycles. Bounded to
128 entries. Ctrl+Z / Ctrl+Y.

## Action kinds

| Kind | Records | Notes |
|---|---|---|
| `Transform` | before/after TRS | one entry per gizmo drag or Details widget release |
| `Spawn` / `Delete` | type, name, id, TRS | respawn re-attaches the same id |
| `Rename` | name before/after | |
| `Lights` | whole `VectorLight` before/after | Environment panel (ambient) edits |
| `Reparent` | parent ids + local TRS before/after | Outliner drag-and-drop |
| `ComponentEdit` | component TypeId + before/after `FieldStore` snapshots | **generic**: covers every current and future component via `Serialize`/`Deserialize` — see [[Serialization & Reflection]] |

History is cleared on scene load (ids die with the old world).

> [!note]
> Undoing a delete respawns the object, but children that were reparented
> when the parent died keep their new parentage.
