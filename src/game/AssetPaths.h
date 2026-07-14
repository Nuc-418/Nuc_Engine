// Engine shader paths, relative to the working directory ($(ProjectDir) = repo
// root). These are the three built-in GLSL programs the sample app loads at
// boot. Everything content-shaped — models, textures, prefabs — is discovered
// at runtime (see ModelDiscovery / the Content Browser), not pinned here.

#pragma once

namespace AssetPaths
{
	constexpr const char* CubeVertexShader        = "assets/shaders/cubeShader/cube.vert";
	constexpr const char* CubeFragmentShader      = "assets/shaders/cubeShader/cube.frag";
	constexpr const char* PrimitiveVertexShader   = "assets/shaders/primitive/primitive.vert";
	constexpr const char* PrimitiveFragmentShader = "assets/shaders/primitive/primitive.frag";
	// Textured-model shader (named for its asset folder); used by every model
	// discovered under assets/models.
	constexpr const char* ModelVertexShader       = "assets/shaders/ironMan/ironMan.vert";
	constexpr const char* ModelFragmentShader     = "assets/shaders/ironMan/ironMan.frag";
}
