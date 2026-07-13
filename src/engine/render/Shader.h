// Shader: a GL program built from vertex+fragment source files.
//
// Replaces the vendored LoadShaders helper. Owns the program object, its
// source paths and a uniform-location cache. Reload() recompiles the sources
// and relinks the SAME program object, so GLuint program ids captured
// elsewhere (spawn factories, MeshRenderer::program) stay valid across a
// reload; a relink can still move uniform locations, so every reload bumps
// GlobalGeneration() and the engine's per-program location caches (Camera,
// Time, Lights) re-query when they see a new generation. Location() is this
// shader's own cached lookup and self-invalidates on reload.
//
// Like Mesh/Texture, GL resources are freed by explicit Unload() while the
// context is current, not by the destructor.

#pragma once

#include <map>
#include <string>
#include <GL/glew.h>

class Shader
{
public:
	// Compiles + links the program. Logs and returns false on failure
	// (the Shader stays empty). Registers the shader for ReloadAll.
	bool Load(const std::string& vertexPath, const std::string& fragmentPath);

	// Recompiles the current sources into the same program id. On any
	// compile/link failure the existing program keeps running unchanged.
	bool Reload();

	// Deletes the program (GL context must be current) and deregisters.
	void Unload();

	GLuint Program() const { return program; }

	// Cached uniform location (-1 when the program doesn't declare it).
	GLint Location(const char* name);

	// Reloads every live Shader (editor "Reload Shaders"). Bumps the
	// generation once so external location caches refresh.
	static void ReloadAll();

	// Incremented on every reload; caches keyed by program id store the
	// generation they were filled at and clear when it changes.
	static unsigned GlobalGeneration();

private:
	bool BuildInto(GLuint targetProgram);

	GLuint program = 0;
	std::string vertexPath;
	std::string fragmentPath;
	std::map<std::string, GLint> locations;
};
