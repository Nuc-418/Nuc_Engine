// Texture: image loading (stb_image) and binding to a shader program.

#pragma once
#include <string>
#include <GL/glew.h>

class Texture
{
public:

	void TextureToProgram(GLuint program, std::string textureFile);

	// Deletes the GL texture. Must run while the GL context is alive.
	void Unload();

private:
	void load_texture(std::string textureFile);

	GLuint textureName = 0;

	
};
