---
tags: [reference]
---

# Source Map

```
NucEngine.sln / .vcxproj      Build files at the repo root ($(ProjectDir) ==
CMakeLists.txt                repo root == the asset-path working directory)
build_and_run.bat             VS build + run helper

src/engine/
  core/      Application (loop, owns everything), Scene interface, Window,
             Time (wall-clock), EngineMath (Euler decompose)
  input/     UserInputs (raw state), InputActions (named bindings),
             Controller (movement)                    → [[Input]]
  scene/     GameObject, Transform, Component, ComponentRegistry, World,
             FieldStore, RotatorComponent, Serialization interfaces
  render/    Camera, Mesh, MeshRenderer, MeshComponent, Material, Texture,
             Lights, LightComponent, CameraComponent, Shader, Primitives,
             Framebuffer                              → [[Rendering]]
  asset/     AssetManager                             → [[Asset System]]
  io/        ObjLoader, ModelDiscovery, SceneSerializer, PrefabLibrary,
             JsonSerialization (shared adapters)
  plugin/    Plugin (EnginePlugin), PluginManager, PluginSort
  editor/    Editor (dockspace/menus), EditorHost (Scene wrapper),
             UndoStack, EditorMath, MeshPreview (thumbnails),
             GamePackager, EditorFileSystem, panels/  → [[Editor Overview]]
  platform/  WindowChrome (Win32 custom title bar)

src/game/    Main.cpp, DemoScene, AssetPaths — the sample application

Plugins/JoltPhysics/   JoltPhysicsPlugin, PhysicsWorld (Jolt pimpl),
                       PhysicsBodyComponent, vendor/Jolt
                                                      → [[Plugin System]]
third_party/  stb, imgui (+ backends), ImGuizmo, nlohmann, doctest,
              GL/GLFW headers, libs/x64 (prebuilt glew32s/glfw3)

assets/       shaders/ (GLSL, loaded at runtime), models/ (auto-discovered),
              scenes/ (saved maps + startup.json), prefabs/
docs/         BUILD, COMPONENTS, EDITOR, PLUGINS, RESTRUCTURE, ROADMAP + this vault
tools/        lin_syntax_check/ (the gate), tests/ (doctest suites)
```
