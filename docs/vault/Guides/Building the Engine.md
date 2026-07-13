---
tags: [guide, build]
---

# Building the Engine

Targets MSVC/Windows x64 (prebuilt GLEW/GLFW libs ship in
`third_party/libs/x64`). The whole tree builds as **C++17 with UTF-8
sources** (`/utf-8`).

## Visual Studio

Open `NucEngine.sln` (VS 2017+ with the v141 toolset) and build x64.
Configurations:

- **Debug/Release** — the editor build (demo scene inside the editor)
- **Game** — standalone game build (`NUC_GAME_BUILD`, target `NucEngineGame`)

> [!warning] GLM
> GLM is expected on the compiler's global include path (e.g. a VC++ user
> props file); it is not vendored.

## CMake

```
cmake -S . -B build
cmake --build build --config Debug
```

Builds one static library `nuc_engine` plus `NucEngine` (editor) and
`NucEngineGame` executables. Engine sources are globbed; Jolt vendored
sources compile into the library.

## Running

Run from the **repo root** — asset paths (`assets/…`) are relative to it.
`build_and_run.bat` automates the VS build + run.

## Packaging

Editor → File → **Package Game**: copies the game executable and assets to
a folder and writes the current world as `assets/scenes/startup.json`,
which the game build loads at boot.

See also [[Testing & Verification Gate]] for the Linux-side checks.
