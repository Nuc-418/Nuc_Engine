# Building NucEngine

There are two equivalent builds. Both compile the same sources and link the
prebuilt GLEW/GLFW in `third_party/libs/x64`; GLM and the other third-party
code are vendored in `third_party/`. The build is Windows/MSVC and x64.

## Visual Studio (NucEngine.vcxproj)

Open `NucEngine.sln` (VS 2017+ with the v141 toolset) and build the x64
configuration. Configurations:

- **Debug / Release** — the editor build (`NucEngine.exe`).
- **Game** — the standalone game build (`NucEngineGame.exe`, defines
  `NUC_GAME_BUILD`); used by **File > Package Game**.

The project pins a specific `WindowsTargetPlatformVersion`; if that SDK is not
installed, retarget the solution or build with
`/p:WindowsTargetPlatformVersion=<installed>` (this is what `build_and_run.bat`
does automatically).

## CMake (CMakeLists.txt)

CMake auto-detects an installed Windows SDK, so it sidesteps the pinned-SDK
issue above.

```
cmake -S . -B build
cmake --build build --config Debug
```

Use a generator matching your CMake version (the "Visual Studio 17 2022"
generator needs CMake ≥ 3.21; older CMake can use `-G "NMake Makefiles"` or
Ninja from a Developer Command Prompt). Targets:

- `NucEngine` — editor build.
- `NucEngineGame` — standalone game build (`NUC_GAME_BUILD`).
- `nuc_engine` — the static library both executables link (engine + third-party
  + the JoltPhysics plugin, including vendored Jolt).

### Language standard and encoding

The whole tree builds as **C++17** with **UTF-8 sources** (`/utf-8`), in both
the `.vcxproj` and `CMakeLists.txt`. (C++17 used to be scoped to the Jolt
translation units because `using namespace std;` in engine headers made
`std::byte` ambiguous with the Windows SDK's `byte` (C2872); those `using`
directives are gone from public headers, so the restriction is too.)

## Running

Run from the repo root (or with it as the working directory) — asset paths are
resolved relative to it. Under Visual Studio the debugger working directory is
already set to the repo root for both the `.vcxproj` and the CMake targets.
