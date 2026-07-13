#include "engine/asset/AssetManager.h"

#include "engine/render/Shader.h"
#include "engine/render/Texture.h"

AssetManager::AssetManager() = default;
AssetManager::~AssetManager() = default;

Shader* AssetManager::LoadShader(const std::string& vertexPath, const std::string& fragmentPath)
{
	std::string key = vertexPath + "|" + fragmentPath;
	auto it = shaders.find(key);
	if (it != shaders.end())
		return it->second.get();

	std::unique_ptr<Shader> shader(new Shader());
	if (!shader->Load(vertexPath, fragmentPath))
		return nullptr;
	return shaders.emplace(key, std::move(shader)).first->second.get();
}

Texture* AssetManager::LoadTexture(GLuint program, const std::string& path)
{
	auto it = textures.find(path);
	if (it != textures.end())
		return it->second.get();

	std::unique_ptr<Texture> texture(new Texture());
	texture->TextureToProgram(program, path);
	return textures.emplace(path, std::move(texture)).first->second.get();
}

void AssetManager::UnloadAll()
{
	for (auto& entry : textures)
		entry.second->Unload();
	textures.clear();
	for (auto& entry : shaders)
		entry.second->Unload();
	shaders.clear();
}
