# NucEngine Restructure

This document describes the restructure of the engine into its current
layout, the design decisions behind it, and the known issues that were
deliberately left untouched.

## Goals

- Separate engine code from game/demo code and from third-party code.
- Remove committed build artifacts and make the repo layout self-explanatory.
- Rename the project and everything in it to `NucEngine`.
- Fix latent bugs and obvious inefficiencies **without changing the demo's
  behavior** — same controls, same visuals.
- Keep the Visual Studio build (v141 toolset). (A CMake build was later added
  alongside it — see `docs/BUILD.md` — but the `.vcxproj` remains.)

## Layout

```
NucEngine.sln / NucEngine.vcxproj   Build files at the repo root, so
                                    $(ProjectDir) == repo root == the working
                                    directory the asset paths are relative to.
src/
  engine/
    core/     Application, Scene, Window, Time, EngineMath
    input/    UserInputs (GLFW callbacks), Controller (WASD/mouse-look)
    render/   Camera, Mesh, MeshRenderer, Material, Texture, Lights
    scene/    GameObject, Transform
    io/       ObjLoader
  game/       Main.cpp, DemoScene, AssetPaths — the sample application
third_party/
  stb/          stb_image.h (vendored)
  libs/x64/     prebuilt glew32s.lib and glfw3.lib
(The LoadShaders helper this restructure originally vendored was later
absorbed into engine/render/Shader.)
assets/
  shaders/    GLSL sources, loaded at runtime
  models/     Iron_Man .obj/.mtl/.tga
```

**Layering rule:** `game` may include `engine` and `third_party`; `engine` may
include `third_party`; never the reverse. Includes are root-qualified
(`#include "engine/render/Mesh.h"`) against two include roots:
`$(ProjectDir)src` and `$(ProjectDir)third_party`.

`math.h` was renamed `EngineMath.h` so a project header can never shadow the
CRT `<math.h>` now that include directories are configured.

## Application / Scene split

`Source.cpp` used to be the whole application: `main()`, shader/scene loading,
per-frame update and draw, all operating on file-scope globals. It is now:

- **`engine/core/Scene.h`** — the interface a game implements:
  `bool Load(Application&)`, `void Update(Application&)`,
  `void Draw(Application&)`, `void Unload(Application&)`.
- **`engine/core/Application`** — owns the `Window`, `UserInputs` and
  `Controller`, performs GLFW/GLEW initialization in `Init`, and drives the
  main loop in `Run(Scene&)`. The loop keeps the original frame ordering
  exactly: `clock()` begin → `Update` → `Draw` → swap → poll → `clock()` end →
  `Time::Update`.
- **`game/DemoScene`** — the former globals as members; `Load`/`Update`/`Draw`
  are near-verbatim moves of the old `loadProgramShader`+`LoadObjects`,
  `Update` and `Draw` functions. Shader failure now returns `false` (clean
  abort) instead of `exit(EXIT_FAILURE)`.
- **`game/Main.cpp`** — the entry point. The console-window positioning and
  cursor hiding are the only Win32-specific lines and sit behind
  `#ifdef _WIN32`.
- **`game/AssetPaths.h`** — every runtime asset path literal in one place.

To add a new scene: implement `Scene`, construct it in `main`, pass it to
`Application::Run`.

### Why explicit `Unload` instead of destructors

`DemoScene` lives on the stack in `main()`. A destructor that deleted GL
objects would run *after* `Application::Shutdown()` has called
`glfwTerminate()`, when no GL context exists. `Application::Run` instead calls
`Scene::Unload` right after the loop ends, while the context is still current.
For the same reason `Mesh` and `Texture` expose `Unload()` methods rather than
destructors (they are also copied around by value in places where destructors
would double-free).

### ESC handling

`Controller::Exit` used to call `glfwDestroyWindow` + `glfwTerminate` +
`exit(0)` mid-frame, skipping all cleanup. It is now
`Controller::RequestExit`, which calls `glfwSetWindowShouldClose`; the loop
ends normally and cleanup runs. Observable behavior is unchanged: ESC closes
the app.

## Behavior-preserving fixes

- **Zero-initialization**: the scene objects used to be globals, which are
  zero-initialized; as class members they are not. `Mesh::VAO` (tested against
  `0` in `LoadVAO`), `Mesh::nVertex`, `MeshRenderer::program`,
  `Transform::position/rotation` and the `UserInputs` members now have
  explicit in-class initializers.
- **ObjLoader**: `fscanf("%s", &std::string)` (undefined behavior) now reads
  into a bounded char buffer; `vt` entries are parsed as the 2-component
  values they are; every path out of `LoadObj` returns a value.
- **Material**: `illum` is parsed with `%u` into its `GLuint` (was `%f`); the
  parser no longer processes a stale line buffer after EOF. The shaders never
  read `material.illum`, so nothing changes visually.
- **Uniform-location caching**: `Camera::CamToProgram` did five
  `glGetProgramResourceLocation` string lookups per object per frame (103
  objects); `Time::TimeToProgram` and the per-frame light toggles did the
  same. Locations are now cached per shader program. Lookups that return `-1`
  (the cube shader only declares `MVP` and `Time`) are cached and passed
  through — GL ignores location `-1`, matching the old behavior. The caches
  are never invalidated because programs are created once at load time and
  never recreated; if that ever changes, the caches must be cleared.
- **Resource cleanup**: meshes, the texture and both shader programs are now
  deleted in `DemoScene::Unload`. Previously nothing was freed and the
  texture's GL name was lost in a local variable.
- **API typos** fixed: `TesxureToProgram` → `TextureToProgram`,
  `AssocieateUserInput` → `AssociateUserInput`, `BasicMoviment` →
  `BasicMovement`.

## Build configuration

- The vcxproj carries the include roots, `GLEW_STATIC` and
  `_CRT_SECURE_NO_WARNINGS` as project-wide preprocessor definitions (they
  were previously `#define`d ad hoc in some headers, and linking only worked
  because `#pragma comment(lib)` was resolved against the project directory).
- Linker inputs (`glew32s.lib`, `glfw3.lib`, `opengl32.lib`) and the library
  directory `third_party\libs\x64` are project settings; the
  `#pragma comment(lib)` lines are gone.
- Win32/x86 configurations were removed — only x64 libraries were ever
  shipped.
- **GLM is still expected on the compiler's global include path** (e.g. a
  VC++ user props file); it is not vendored. This is unchanged from before.
- `LoadShaders.h` no longer force-defines `_DEBUG`; MSVC defines it in Debug
  configurations, so shader compile/link error logs appear in Debug builds
  and are compiled out of Release builds.

## Previously-known issues now fixed

These four issues were once documented as "intentionally left alone" because
fixing them changes observable behavior. They have now been corrected:

- `Texture::load_texture` bound the texture name to `GL_TEXTURE_CUBE_MAP` but
  uploaded and parameterized `GL_TEXTURE_2D`. It now binds `GL_TEXTURE_2D`, so
  the bind target matches the upload and parameter calls.
- The main loop measured the frame with `clock()` (CPU time), so
  `Time::deltaTime` under-reported real frame time. It now uses
  `glfwGetTime()` (wall time), which reports true elapsed time — animation and
  movement that scale with `deltaTime` now advance at real-world rates.
- `Camera` stored the normal matrix in a `glm::mat4` and uploaded it with
  `glProgramUniformMatrix3fv`, so the GPU received the first 9 floats of a 4x4
  column layout rather than a packed 3x3. `normalMatrix` is now a `glm::mat3`,
  so the upload sends a contiguous, correct 3x3 (matching the pattern already
  used in `MeshPreview`). This corrects the shading normals.
- `UserInputs` was Windows-only (`GetCursorPos`/`SetCursorPos` on screen
  coordinates via `<windows.h>`). It now uses GLFW's cursor API
  (`glfwGetCursorPos`/`glfwSetCursorPos`) in content-area coordinates, so the
  input layer builds and runs on any GLFW platform. (The file-static
  instance pointer it still used was later replaced by the GLFW window user
  pointer, alongside the named-action input rework.)
