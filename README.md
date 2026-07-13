# NucEngine

A small OpenGL 3D engine written in C++ (GLFW + GLEW + GLM) with a UE5-style
in-app editor (Dear ImGui + ImGuizmo), and a demo scene featuring a cube grid,
an indexed cube, two textured Iron Man models and four toggleable light
sources. See `docs/EDITOR.md` for the editor guide.

## Layout

```
src/engine/     Engine code (core, input, render, scene, io, plugin)
src/game/       The demo application (Main.cpp, DemoScene, AssetPaths)
Plugins/        Self-contained engine plugins (JoltPhysics)
third_party/    Vendored code and prebuilt libs (stb_image, LoadShaders, GLEW/GLFW)
assets/         Runtime assets (GLSL shaders, models)
docs/           RESTRUCTURE.md (layout), COMPONENTS.md (actor/component model),
                PLUGINS.md (plugin system), BUILD.md, EDITOR.md
```

## Scene model

Objects are UE5-style **actors**: a `GameObject` is a `Transform` plus a list of
`Component`s (mesh today; physics, lights and prefabs build on this). See
`docs/COMPONENTS.md`.

## Plugins

Optional features live outside engine core as plugins under `Plugins/`, hooked
in through a small `EnginePlugin` interface (`src/engine/plugin/`). The first is
**JoltPhysics**, 3D rigid-body physics backed by [Jolt](https://github.com/jrouwe/JoltPhysics).
See `docs/PLUGINS.md` and `Plugins/JoltPhysics/README.md`. The demo drops a
cube onto a floor — visible in the standalone game and in the editor's Play mode.

## Building

Two equivalent builds are provided; both compile the same sources and link the
prebuilt GLEW/GLFW in `third_party/libs/x64` (GLM is vendored in `third_party`).

**Visual Studio:** open `NucEngine.sln` (2017 or later, v141 toolset) and build
the x64 configuration.

**CMake** (alternative; auto-detects the installed Windows SDK):

```
cmake -S . -B build
cmake --build build --config Debug
```

It produces two executables — `NucEngine` (editor build) and `NucEngineGame`
(standalone game, `NUC_GAME_BUILD`). See `docs/BUILD.md` for details.

Run from Visual Studio (the debugger's working directory is the repo root) or
run the built exe with the repo root as the working directory — asset paths are
resolved relative to it.

## Controls

The app starts in the **editor** (see `docs/EDITOR.md`): RMB to fly, W/E/R
gizmos, F to focus, Ctrl+S to save the scene, **[ Play ]** to run the demo,
and **File > Package Game** to export a standalone build (requires building
the Game|x64 configuration once).
In **Play mode** (Esc returns to the editor):

- **1** — Toggle ambient light
- **2** — Toggle directional light
- **3** — Toggle point light
- **4** — Toggle spot light
- **5** — Toggle distortion shader
- **6/7/8/9** — Render mode: triangles / line strip / points / triangle fan
- **W, A, S, D** — Move the camera
- **Space / Ctrl** — Move up / down
- **Shift** — Speed boost
- **Mouse** — Look around
- **Esc** — Quit
