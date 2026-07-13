---
tags: [guide, testing]
---

# Testing & Verification Gate

The real build is MSVC/Windows; `tools/lin_syntax_check/check.sh` is a
Linux-side gate that verifies every change without a Windows box. Run it
from anywhere; it must end with `ALL CHECKS PASSED`.

## What it checks

1. **Syntax** — `g++ -fsyntax-only -std=c++17` on every vcxproj ClCompile
   item (MSBuild `**` globs expanded; vendored Jolt TUs skipped — the
   plugin's own TUs cover the include seam). System GLFW/GLEW/GLM headers
   plus a `windows.h` stub.
2. **Build-file consistency** — vcxproj ⟷ filters ⟷ filesystem: every
   tracked source under `src/` and `Plugins/` (vendor trees exempt) must be
   in the project; every project item must exist on disk.
3. **Grep gates** — layering and integration hazards (engine must not
   include game headers, single key-callback site, no GLEW in vendored
   imgui, …).
4. **Game-build branch** — `Main.cpp`/`DemoScene.cpp` also compile with
   `NUC_GAME_BUILD`.
5. **Euler decompose self-test** — numeric check of
   `EulerYXZFromMatrix` against the engine's rotation convention.
6. **Unit tests** — doctest (vendored, `third_party/doctest`), sources in
   `tools/tests/`: Transform math and hierarchy rebasing, component
   serialization round-trips, primitive mesh invariants, plugin dependency
   sorting, input actions.

## Adding tests

Add a `tools/tests/test_*.cpp` and append it (plus any newly needed
GL-free engine `.cpp`) to the unit-test compile line in `check.sh`. Keep
test dependencies GL-free — the test binary links no GL/GLEW.

> [!important]
> New source files must be added to `NucEngine.vcxproj` **and**
> `NucEngine.vcxproj.filters` (and `CMakeLists.txt` for plugins) or the
> consistency check fails.
