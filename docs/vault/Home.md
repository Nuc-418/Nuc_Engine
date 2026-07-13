---
tags: [moc]
---

# NucEngine

A modular C++17 game engine with an in-app, UE5-style editor. OpenGL
rendering, GLFW windowing, Dear ImGui editor UI, JoltPhysics via the plugin
system.

> [!tip] New here?
> Read [[Architecture Overview]] first, then [[GameObject & Hierarchy]] and
> [[Components]]. To build and run, see [[Building the Engine]].

## Map of content

### Architecture
- [[Architecture Overview]] — layering, module map, design rules
- [[Application & Main Loop]] — bootstrap, the frame, shutdown order
- [[World]] — the object registry and scene state
- [[GameObject & Hierarchy]] — actors, parenting, world transforms
- [[Components]] — the component model and its lifecycle
- [[Serialization & Reflection]] — ISerializer, scene files, property UI
- [[Plugin System]] — modules, dependencies, JoltPhysics
- [[Rendering]] — meshes, cameras, lights, shaders
- [[Asset System]] — AssetManager, primitives, model discovery
- [[Input]] — named actions over raw key state

### Built-in components
[[MeshComponent]] · [[LightComponent]] · [[CameraComponent]] ·
[[RotatorComponent]] · [[PhysicsBodyComponent]]

### Editor
- [[Editor Overview]] — layout, panels, Play-in-Editor
- [[Undo System]] — what is undoable and how
- [[Prefabs]] — objects as reusable assets
- [[Controls & Keybindings]]

### Guides
- [[Building the Engine]]
- [[Adding a Component Type]]
- [[Writing a Plugin]]
- [[Adding Meshes & Models]]
- [[Scene File Format]]
- [[Testing & Verification Gate]]

### Reference
- [[Source Map]] — every directory and what lives in it
- [[Roadmap Status]] — what is done, deferred, and next
