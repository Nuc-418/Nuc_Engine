// AssetManager: the engine's owner of shared GPU-backed assets.
//
// One instance lives on Application (app.assets). Assets are keyed by their
// source path(s): repeated Load* calls return the same object instead of
// re-reading and re-uploading. The manager frees everything in UnloadAll,
// which Application::Run calls after the scene and plugins unload, while the
// GL context is still current — scenes no longer delete shaders or textures
// by hand.

#pragma once

#include <map>
#include <memory>
#include <string>
#include <GL/glew.h>

class Shader;
class Texture;

class AssetManager
{
public:
	// Defined in the .cpp: the unique_ptr maps need complete Shader/Texture
	// types to destroy, and this header only forward-declares them.
	AssetManager();
	~AssetManager();

	// Compiles/links (or returns the cached) shader for this source pair.
	// Null on compile/link failure (logged).
	Shader* LoadShader(const std::string& vertexPath, const std::string& fragmentPath);

	// Loads (or returns the cached) texture. On first load it is bound to
	// `program`'s sampler, matching Texture::TextureToProgram.
	Texture* LoadTexture(GLuint program, const std::string& path);

	// Frees every asset's GL objects (context must be current) and empties
	// the caches.
	void UnloadAll();

private:
	std::map<std::string, std::unique_ptr<Shader>> shaders;
	std::map<std::string, std::unique_ptr<Texture>> textures;
};
