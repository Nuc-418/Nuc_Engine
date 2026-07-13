# NucEngine - CMake build.
#
# This is an alternative to the Visual Studio project (NucEngine.vcxproj); both
# build the same sources. CMake auto-detects an installed Windows SDK, so it
# avoids the SDK version the .vcxproj pins.
#
# The engine, third-party sources and the JoltPhysics plugin (including the
# vendored Jolt library) build into one static library, nuc_engine. Two thin
# executables link it:
#   NucEngine      - the editor build (demo scene inside the in-app editor)
#   NucEngineGame  - the standalone game build (defines NUC_GAME_BUILD)
#
# Configure + build (from the repo root, in a VS/MSVC environment), e.g.:
#   cmake -S . -B build
#   cmake --build build --config Debug
# Run the executables from the repo root so relative asset paths resolve.

cmake_minimum_required(VERSION 3.14)
project(NucEngine CXX)

if(NOT MSVC)
	message(WARNING
		"NucEngine currently targets MSVC/Windows: GLEW and GLFW are shipped as "
		"prebuilt x64 libs in third_party/libs/x64, and some input code is Win32-only.")
endif()

# NOTE: C++17 is applied per-file (to Jolt + the physics wrapper) further down,
# NOT globally. The engine uses `using namespace std;` together with <windows.h>,
# and under C++17 that makes std::byte ambiguous with the Windows SDK's `byte`
# (C2872). Everything else builds as the MSVC default (C++14), mirroring the
# .vcxproj, which sets stdcpp17 only on the Jolt sources.

# Default single-config generators to Debug.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
# Engine code is globbed (CONFIGURE_DEPENDS re-globs when files change).
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS ${REPO_ROOT}/src/engine/*.cpp)

# Vendored third-party sources compiled into the project (headers are elsewhere).
set(THIRD_PARTY_SOURCES
	${REPO_ROOT}/third_party/LoadShaders/LoadShaders.cpp
	${REPO_ROOT}/third_party/imgui/imgui.cpp
	${REPO_ROOT}/third_party/imgui/imgui_demo.cpp
	${REPO_ROOT}/third_party/imgui/imgui_draw.cpp
	${REPO_ROOT}/third_party/imgui/imgui_tables.cpp
	${REPO_ROOT}/third_party/imgui/imgui_widgets.cpp
	${REPO_ROOT}/third_party/imgui/backends/imgui_impl_glfw.cpp
	${REPO_ROOT}/third_party/imgui/backends/imgui_impl_opengl3.cpp
	${REPO_ROOT}/third_party/ImGuizmo/ImGuizmo.cpp
)

# JoltPhysics plugin wrapper + the vendored Jolt library sources.
set(PLUGIN_SOURCES
	${REPO_ROOT}/Plugins/JoltPhysics/PhysicsWorld.cpp
	${REPO_ROOT}/Plugins/JoltPhysics/JoltPhysicsPlugin.cpp
)
file(GLOB_RECURSE JOLT_SOURCES CONFIGURE_DEPENDS ${REPO_ROOT}/Plugins/JoltPhysics/vendor/Jolt/*.cpp)

# Only Jolt and the physics wrapper (the TUs that include Jolt) are built as
# C++17; see the std::byte note above.
set(CXX17_SOURCES
	${REPO_ROOT}/Plugins/JoltPhysics/PhysicsWorld.cpp
	${JOLT_SOURCES}
)
if(MSVC)
	set_source_files_properties(${CXX17_SOURCES} PROPERTIES COMPILE_FLAGS "/std:c++17")
else()
	set_source_files_properties(${CXX17_SOURCES} PROPERTIES COMPILE_FLAGS "-std=c++17")
endif()

# ---------------------------------------------------------------------------
# Core static library: everything that does not depend on NUC_GAME_BUILD.
# ---------------------------------------------------------------------------
add_library(nuc_engine STATIC
	${ENGINE_SOURCES}
	${THIRD_PARTY_SOURCES}
	${PLUGIN_SOURCES}
	${JOLT_SOURCES}
)

target_include_directories(nuc_engine PUBLIC
	${REPO_ROOT}/src
	${REPO_ROOT}/third_party
	${REPO_ROOT}/third_party/imgui
	${REPO_ROOT}/Plugins
	${REPO_ROOT}/Plugins/JoltPhysics/vendor
)

target_compile_definitions(nuc_engine PUBLIC
	GLM_FORCE_CTOR_INIT
	NOMINMAX
	GLEW_STATIC
	_CRT_SECURE_NO_WARNINGS
)

# Prebuilt static libs shipped in the repo, plus the system OpenGL lib.
set(NUC_LIB_DIR ${REPO_ROOT}/third_party/libs/x64)
target_link_libraries(nuc_engine PUBLIC
	${NUC_LIB_DIR}/glew32s.lib
	${NUC_LIB_DIR}/glfw3.lib
	opengl32
)

# ---------------------------------------------------------------------------
# Executables: same game sources, differing only by NUC_GAME_BUILD.
# ---------------------------------------------------------------------------
set(GAME_SOURCES
	${REPO_ROOT}/src/game/Main.cpp
	${REPO_ROOT}/src/game/DemoScene.cpp
)

add_executable(NucEngine ${GAME_SOURCES})
target_link_libraries(NucEngine PRIVATE nuc_engine)

add_executable(NucEngineGame ${GAME_SOURCES})
target_compile_definitions(NucEngineGame PRIVATE NUC_GAME_BUILD)
target_link_libraries(NucEngineGame PRIVATE nuc_engine)

# Asset paths are relative to the repo root; make VS debugging run from there.
set_target_properties(NucEngine NucEngineGame PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY ${REPO_ROOT}
)
