// Texture: image loading (stb_image) and binding to a shader program.

#pragma once
#include <iostream>
#include <GL/glew.h>



using namespace std;
class Texture
{
public:

	void TextureToProgram(GLuint program, string textureFile);

private:
	void load_texture(string textureFile);

	
};
