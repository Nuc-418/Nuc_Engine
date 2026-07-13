// All runtime asset paths, relative to the working directory ($(ProjectDir) = repo root).

#pragma once

namespace AssetPaths
{
	constexpr const char* CubeVertexShader     = "assets/shaders/cubeShader/cube.vert";
	constexpr const char* CubeFragmentShader   = "assets/shaders/cubeShader/cube.frag";
	constexpr const char* PrimitiveVertexShader = "assets/shaders/primitive/primitive.vert";
	constexpr const char* PrimitiveFragmentShader = "assets/shaders/primitive/primitive.frag";
	constexpr const char* IronManVertexShader  = "assets/shaders/ironMan/ironMan.vert";
	constexpr const char* IronManFragmentShader = "assets/shaders/ironMan/ironMan.frag";
	constexpr const char* IronManFolder        = "assets/models/Iron_Man/";
	constexpr const char* IronManObjFile       = "Iron_Man.obj";
	constexpr const char* IronManTexture       = "assets/models/Iron_Man/Iron_Man_D.tga";
}
