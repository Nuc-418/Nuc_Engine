# NucEngine

A small OpenGL 3D engine written in C++ (GLFW + GLEW + GLM) with a UE5-style
in-app editor (Dear ImGui + ImGuizmo), and a demo scene featuring a cube grid,
an indexed cube, two textured Iron Man models and four toggleable light
sources. See `docs/EDITOR.md` for the editor guide.

## Layout

```
src/engine/     Engine code (core, input, render, scene, io)
src/game/       The demo application (Main.cpp, DemoScene, AssetPaths)
third_party/    Vendored code and prebuilt libs (stb_image, LoadShaders, GLEW/GLFW)
assets/         Runtime assets (GLSL shaders, models)
docs/           docs/RESTRUCTURE.md explains the structure and its rationale
```

## Building

Open `NucEngine.sln` in Visual Studio (2017 or later with the v141 toolset)
and build the x64 configuration. GLM must be available on the compiler's
include path; GLEW and GLFW are shipped prebuilt in `third_party/libs/x64`.

Run from Visual Studio (the debugger's working directory, `$(ProjectDir)`,
is the repo root) or run the built exe with the repo root as the working
directory — asset paths are resolved relative to it.

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
