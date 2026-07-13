// Shader: compile/link (absorbed from the old LoadShaders helper), uniform
// location caching, and stable-id hot reload.

#include "engine/render/Shader.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace {

// Live shaders, for ReloadAll. Registration keys off Load/Unload (not
// ctor/dtor) so only shaders that actually own a program are tracked.
std::vector<Shader*>& Registry()
{
	static std::vector<Shader*> shaders;
	return shaders;
}

unsigned generation = 0;

bool ReadFileText(const std::string& path, std::string& out)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) {
		std::cout << "Shader: cannot open " << path << std::endl;
		return false;
	}
	std::ostringstream text;
	text << file.rdbuf();
	out = text.str();
	return true;
}

// Compiles one shader stage; 0 (with a log) on failure.
GLuint CompileStage(GLenum type, const std::string& path)
{
	std::string source;
	if (!ReadFileText(path, source))
		return 0;

	GLuint shader = glCreateShader(type);
	const GLchar* text = source.c_str();
	glShaderSource(shader, 1, &text, NULL);
	glCompileShader(shader);

	GLint compiled = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		std::vector<GLchar> log(length + 1, 0);
		glGetShaderInfoLog(shader, length, &length, log.data());
		std::cout << "Shader compile failed (" << path << "):\n" << log.data() << std::endl;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

} // namespace

// Compiles both stages and links them into targetProgram. Any previously
// attached shaders are detached first, so this works for both first load and
// reload. Returns false (program left as-is where possible) on failure.
bool Shader::BuildInto(GLuint targetProgram)
{
	GLuint vert = CompileStage(GL_VERTEX_SHADER, vertexPath);
	if (!vert)
		return false;
	GLuint frag = CompileStage(GL_FRAGMENT_SHADER, fragmentPath);
	if (!frag) {
		glDeleteShader(vert);
		return false;
	}

	// Detach whatever is currently linked (reload path).
	GLint attachedCount = 0;
	glGetProgramiv(targetProgram, GL_ATTACHED_SHADERS, &attachedCount);
	if (attachedCount > 0) {
		std::vector<GLuint> attached(attachedCount);
		glGetAttachedShaders(targetProgram, attachedCount, NULL, attached.data());
		for (GLuint old : attached) {
			glDetachShader(targetProgram, old);
			glDeleteShader(old);
		}
	}

	glAttachShader(targetProgram, vert);
	glAttachShader(targetProgram, frag);
	glLinkProgram(targetProgram);
	// The program keeps the stages alive; drop our references.
	glDeleteShader(vert);
	glDeleteShader(frag);

	GLint linked = GL_FALSE;
	glGetProgramiv(targetProgram, GL_LINK_STATUS, &linked);
	if (!linked) {
		GLint length = 0;
		glGetProgramiv(targetProgram, GL_INFO_LOG_LENGTH, &length);
		std::vector<GLchar> log(length + 1, 0);
		glGetProgramInfoLog(targetProgram, length, &length, log.data());
		std::cout << "Shader link failed (" << vertexPath << " + " << fragmentPath << "):\n"
		          << log.data() << std::endl;
		return false;
	}
	return true;
}

bool Shader::Load(const std::string& vertPath, const std::string& fragPath)
{
	vertexPath = vertPath;
	fragmentPath = fragPath;

	// Build into a fresh program so a failed Load leaves no half-built state.
	GLuint candidate = glCreateProgram();
	if (!BuildInto(candidate)) {
		glDeleteProgram(candidate);
		return false;
	}

	if (program != 0)
		Unload();
	program = candidate;
	locations.clear();
	Registry().push_back(this);
	std::cout << "Shader loaded: " << vertexPath << " + " << fragmentPath
	          << " (program " << program << ")" << std::endl;
	return true;
}

bool Shader::Reload()
{
	if (program == 0)
		return false;

	// Validate the new sources in a throwaway program first, so a broken
	// edit never bricks the live one.
	GLuint probe = glCreateProgram();
	bool ok = BuildInto(probe);
	glDeleteProgram(probe);
	if (!ok)
		return false;

	if (!BuildInto(program))
		return false; // unreachable in practice: the probe just linked

	locations.clear();
	generation++;
	std::cout << "Shader reloaded: " << vertexPath << " + " << fragmentPath
	          << " (program " << program << ")" << std::endl;
	return true;
}

void Shader::Unload()
{
	if (program == 0)
		return;
	glDeleteProgram(program);
	program = 0;
	locations.clear();
	std::vector<Shader*>& registry = Registry();
	registry.erase(std::remove(registry.begin(), registry.end(), this), registry.end());
}

GLint Shader::Location(const char* name)
{
	auto it = locations.find(name);
	if (it == locations.end())
		it = locations.emplace(name, glGetProgramResourceLocation(program, GL_UNIFORM, name)).first;
	return it->second;
}

void Shader::ReloadAll()
{
	for (Shader* shader : Registry())
		shader->Reload();
}

unsigned Shader::GlobalGeneration()
{
	return generation;
}
