---
tags: [reference, roadmap]
---

# Roadmap Status

The full plan lives in `docs/ROADMAP.md` (repo). Summary as of 2026-07:

| Phase | Status |
|---|---|
| **0 — Foundations** | ✅ Done: verification gate repaired, C++17 + `/utf-8` project-wide, headers de-polluted, UTF-8 sources enforced, doctest harness |
| **1 — Scene graph** | ✅ Done: parent/child hierarchy with keep-world reparenting, [[LightComponent]], [[CameraComponent]], RTTI-free GetComponent. *Quaternion Transform storage deferred* (Euler + tested decompose covers current needs) |
| **2 — Asset system** | ✅ Core: [[Asset System|AssetManager]], Shader asset with stable-id hot reload, engine Primitives, std::filesystem model discovery. *Mesh handles arrive with Phase 3* |
| **3 — Renderer rework** | ⏸ Not started — retained render queue, UBOs, GL choke point. **Needs on-hardware pixel comparison**; the plan keeps the old uniform path compiled until verified |
| **4 — Plugin system v2** | ✅ Core: explicit `RegisterTypes`, Version/Dependencies, dependency-sorted lifecycle, [[PhysicsBodyComponent]], `ServiceRegistry` (interface-keyed; `IPhysicsService` provided by JoltPhysics). *Stretch remaining: DLL loading* |
| **5 — Gameplay & input** | ✅ [[Input|Named input actions]] (+ gamepad via GLFW joystick API), behavior hooks + [[RotatorComponent]], `BehaviorContext` gives behaviors input access. *Stretch remaining: Lua scripting plugin* |
| **6 — Editor workflow** | ✅ Done: [[Serialization & Reflection|Property reflection]] UI, generic component [[Undo System|undo]], [[Prefabs]], editor flag-soup → `EditorCommandQueue` (menus/panels enqueue intents, `EditorHost` drains them) |
| **7 — Platform/build** | ⏸ Not started — CMake as source of truth, platform seams for a Linux build. Needs a direction decision (the gate currently parses the vcxproj) |
