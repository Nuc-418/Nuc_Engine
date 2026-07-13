#!/usr/bin/env bash
# Linux-side verification gate for NucEngine (the real build is MSVC on Windows).
# 1. g++ -fsyntax-only on every vcxproj ClCompile item (system GLFW/GLEW/GLM
#    headers + a windows.h stub).
# 2. vcxproj <-> filters <-> filesystem consistency.
# 3. Grep gates for known integration hazards.
set -u
cd "$(dirname "$0")/../.."
ROOT=$PWD
STUBS=tools/lin_syntax_check/stubs
FAIL=0

# ClCompile items with MSBuild ** globs expanded. The vendored Jolt sources
# are skipped in the per-TU loop (hundreds of TUs, third-party code); the
# plugin's own TUs, which include Jolt headers, still compile and cover the
# include seam.
mapfile -t SOURCES < <(python3 - <<'PY'
import glob, re
for item in re.findall(r'<ClCompile Include="([^"]+)"', open('NucEngine.vcxproj', encoding='utf-8').read()):
    path = item.replace('\\', '/')
    for f in (glob.glob(path, recursive=True) if '*' in path else [path]):
        if 'vendor/Jolt' not in f:
            print(f)
PY
)

CXXFLAGS=(-fsyntax-only -std=c++17
          -DGLEW_STATIC -D_CRT_SECURE_NO_WARNINGS
          -I"$STUBS" -Isrc -Ithird_party -Ithird_party/imgui
          -IPlugins -IPlugins/JoltPhysics/vendor)

echo "== syntax check (${#SOURCES[@]} translation units; vendored Jolt skipped) =="
for f in "${SOURCES[@]}"; do
  if ! g++ "${CXXFLAGS[@]}" "$f" 2>/tmp/lin_check_err.txt; then
    echo "FAIL $f"; head -15 /tmp/lin_check_err.txt; FAIL=1
  fi
done
[ $FAIL -eq 0 ] && echo "all TUs OK"

echo "== build-file consistency =="
python3 - <<'PY' || FAIL=1
import glob, re, os, subprocess, sys
def items(p):
    return sorted(re.findall(r'<(?:ClCompile|ClInclude|None) Include="([^"]+)"', open(p, encoding='utf-8').read()))
def expand(p):
    path = p.replace('\\', '/')
    return glob.glob(path, recursive=True) if '*' in path else [path]
vc, fl = items('NucEngine.vcxproj'), items('NucEngine.vcxproj.filters')
ok = True
if vc != fl:
    ok = False; print('vcxproj/filters mismatch:', set(vc) ^ set(fl))
vcset = set()
for p in vc:
    files = expand(p)
    if not files:
        ok = False; print('missing on disk:', p)
    vcset.update(files)
# Every first-party source must be in the project. third_party and plugin
# vendor trees are exempt (vendored code is listed only as far as it is built).
tracked = subprocess.check_output(['git', 'ls-files', 'src', 'Plugins'], text=True).split()
for f in tracked:
    if f.endswith(('.cpp', '.h')) and '/vendor/' not in f and f not in vcset:
        ok = False; print('not in vcxproj:', f)
print('consistency OK' if ok else 'consistency FAILED')
sys.exit(0 if ok else 1)
PY

echo "== grep gates =="
gate() { # gate <description> <max-count> <grep args...>
  local desc=$1 max=$2; shift 2
  local n; n=$(grep -rn "$@" 2>/dev/null | wc -l)
  if [ "$n" -gt "$max" ]; then echo "GATE FAIL: $desc ($n > $max)"; grep -rn "$@" | head -5; FAIL=1; fi
}
gate "glfwSetKeyCallback set in exactly one engine file" 1 --include='*.cpp' --include='*.h' 'glfwSetKeyCallback' src
gate "no GLEW inside vendored imgui" 0 -r 'GL/glew.h' third_party/imgui
gate "no legacy imgui loader defines" 0 -r 'IMGUI_IMPL_OPENGL_LOADER' src
gate "engine must not include game headers" 0 -r --include='*.cpp' --include='*.h' '#include "game/' src/engine
gate "no TBP3D remnants" 0 -rI 'TBP3D' src third_party assets docs README.md NucEngine.sln NucEngine.vcxproj NucEngine.vcxproj.filters

echo "== game-build (NUC_GAME_BUILD) branch =="
for f in src/game/Main.cpp src/game/DemoScene.cpp; do
  if ! g++ "${CXXFLAGS[@]}" -DNUC_GAME_BUILD "$f" 2>/tmp/lin_check_err.txt; then
    echo "FAIL (game build) $f"; head -15 /tmp/lin_check_err.txt; FAIL=1
  fi
done

echo "== euler decompose self-test =="
if g++ -std=c++17 -Isrc -Ithird_party -o /tmp/nuc_euler_test tools/lin_syntax_check/euler_test.cpp src/engine/editor/EditorMath.cpp src/engine/core/EngineMath.cpp && /tmp/nuc_euler_test; then :; else FAIL=1; fi

echo "== unit tests (doctest) =="
if g++ -std=c++17 -DGLM_FORCE_CTOR_INIT -Isrc -Ithird_party \
     -o /tmp/nuc_unit_tests tools/tests/main.cpp tools/tests/test_transform.cpp tools/tests/test_light_component.cpp tools/tests/test_primitives.cpp tools/tests/test_plugin_sort.cpp \
     src/engine/scene/Transform.cpp src/engine/core/EngineMath.cpp src/engine/render/LightComponent.cpp src/engine/render/CameraComponent.cpp src/engine/scene/ComponentRegistry.cpp src/engine/render/Primitives.cpp src/engine/plugin/PluginSort.cpp \
   && /tmp/nuc_unit_tests; then :; else FAIL=1; fi

if [ $FAIL -eq 0 ]; then echo "ALL CHECKS PASSED"; else echo "CHECKS FAILED"; fi
exit $FAIL
