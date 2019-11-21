#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <GLFW\glfw3.h>
#include "stb_image.h"

void Texture::load_texture(string textureFile) {
	GLuint textureName = 0;

	// Gera um nome de textura
	glGenTextures(1, &textureName);

	// Ativa a Unidade de Textura #0
	glActiveTexture(GL_TEXTURE0);

	// Vincula esse nome de textura ao target GL_TEXTURE_CUBE_MAP da Unidade de Textura ativa.
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureName);

	// Ativa a inversão vertical da imagem, aquando da sua leitura para memória.
	stbi_set_flip_vertically_on_load(true);


	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	


	// Leitura/descompressão do ficheiro com imagem de textura
	int width, height, nChannels;
	unsigned char *imageData = stbi_load(textureFile.c_str(), &width, &height, &nChannels, 0);
	if (imageData) {
		// Carrega os dados da imagem para o Objeto de Textura vinculado ao target da face
		glTexImage2D(GL_TEXTURE_2D,
			0,					// Nível do Mipmap
			GL_RGB,				// Formato interno do OpenGL
			width, height,		// width, height
			0,					// border
			nChannels == 4 ? GL_RGBA : GL_RGB,	// Formato da imagem
			GL_UNSIGNED_BYTE,	// Tipos dos dados da imagem
			imageData);			// Apontador para os dados da imagem de textura


		// Liberta a imagem da memória do CPU
		stbi_image_free(imageData);

		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1);
		
	}
	else {
		cout << "Error loading texture!" << endl;
	}

}

void Texture::TesxureToProgram(GLuint program, string textureFile)
{
	load_texture(textureFile);
	GLint location_textureArray = glGetProgramResourceLocation(program, GL_UNIFORM, "textureMap");
	glProgramUniform1i(program, location_textureArray, 0 /* Unidade de Textura #0 */);
	
	
	cout << "Texture Loaded to program :  "<< program << endl;
}