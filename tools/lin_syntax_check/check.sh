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

mapfile -t SOURCES < <(grep -oP '<ClCompile Include="\K[^"]+' NucEngine.vcxproj | tr '\\' '/')

echo "== syntax check (${#SOURCES[@]} translation units) =="
for f in "${SOURCES[@]}"; do
  if ! g++ -fsyntax-only -std=c++14 -finput-charset=latin1 \
       -DGLEW_STATIC -D_CRT_SECURE_NO_WARNINGS \
       -I"$STUBS" -Isrc -Ithird_party -Ithird_party/imgui \
       "$f" 2>/tmp/lin_check_err.txt; then
    echo "FAIL $f"; head -15 /tmp/lin_check_err.txt; FAIL=1
  fi
done
[ $FAIL -eq 0 ] && echo "all TUs OK"

echo "== build-file consistency =="
python3 - <<'PY' || FAIL=1
import re, os, subprocess, sys
def items(p):
    return sorted(re.findall(r'<(?:ClCompile|ClInclude|None) Include="([^"]+)"', open(p, encoding='utf-8').read()))
vc, fl = items('NucEngine.vcxproj'), items('NucEngine.vcxproj.filters')
ok = True
if vc != fl:
    ok = False; print('vcxproj/filters mismatch:', set(vc) ^ set(fl))
for p in vc:
    if not os.path.exists(p.replace('\\', '/')):
        ok = False; print('missing on disk:', p)
tracked = subprocess.check_output(['git', 'ls-files', 'src', 'third_party'], text=True).split()
vcset = {p.replace('\\', '/') for p in vc}
for f in tracked:
    if f.endswith(('.cpp', '.h')) and f not in vcset:
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
gate "no std::filesystem (v141/C++14)" 0 -r --include='*.cpp' --include='*.h' '<filesystem>' src
gate "engine must not include game headers" 0 -r --include='*.cpp' --include='*.h' '#include "game/' src/engine
gate "no TBP3D remnants" 0 -rI 'TBP3D' src third_party assets docs README.md NucEngine.sln NucEngine.vcxproj NucEngine.vcxproj.filters

echo "== game-build (NUC_GAME_BUILD) branch =="
for f in src/game/Main.cpp src/game/DemoScene.cpp; do
  if ! g++ -fsyntax-only -std=c++14 -finput-charset=latin1 \
       -DNUC_GAME_BUILD -DGLEW_STATIC -D_CRT_SECURE_NO_WARNINGS \
       -I"$STUBS" -Isrc -Ithird_party -Ithird_party/imgui \
       "$f" 2>/tmp/lin_check_err.txt; then
    echo "FAIL (game build) $f"; head -15 /tmp/lin_check_err.txt; FAIL=1
  fi
done

echo "== euler decompose self-test =="
if g++ -std=c++14 -Isrc -o /tmp/nuc_euler_test tools/lin_syntax_check/euler_test.cpp src/engine/editor/EditorMath.cpp && /tmp/nuc_euler_test; then :; else FAIL=1; fi

if [ $FAIL -eq 0 ]; then echo "ALL CHECKS PASSED"; else echo "CHECKS FAILED"; fi
exit $FAIL
