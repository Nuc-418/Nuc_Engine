#include"Material.h"
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include <GL\glew.h>


void Material::loadMaterial(char* path)
{
	FILE *file;


	file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
	}
	else
	{
		std::vector<unsigned int>vertexPosElements, vertexUvElements, vertexNormalElements;
		std::vector<glm::vec3>vertexPos;
		std::vector<glm::vec2>vertexUvs;
		std::vector<glm::vec3>vertexNormals;

		while (!loaded)
		{
			char lineHeader[128];
			// read the first word of the line
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
			{
				fclose(file);
				loaded = true;
			}

			if (strcmp(lineHeader, "Ns") == 0)
			{
				GLfloat ns;
				fscanf(file, "%f\n", &ns);
				materialInfo.shininess = ns;
			}
			else if (strcmp(lineHeader, "Ka") == 0)
			{
				glm::vec3 ambient;
				fscanf(file, "%f %f %f\n", &ambient.x, &ambient.y, &ambient.z);
				materialInfo.ambient = ambient;
			}
			else if (strcmp(lineHeader, "Kd") == 0)
			{
				glm::vec3 diffuse;
				fscanf(file, "%f %f %f\n", &diffuse.x, &diffuse.y, &diffuse.z);
				materialInfo.diffuse = diffuse;
			}
			else if (strcmp(lineHeader, "Ks") == 0)
			{
				glm::vec3 specular;
				fscanf(file, "%f %f %f\n", &specular.x, &specular.y, &specular.z);
				materialInfo.specular = specular;
			}
			else if (strcmp(lineHeader, "Ni") == 0)
			{
				GLfloat ni;
				fscanf(file, "%f\n", &ni);
				materialInfo.opticalDensity = ni;
			}
			else if (strcmp(lineHeader, "d") == 0)
			{
				GLfloat d;
				fscanf(file, "%f\n", &d);
				materialInfo.alpha = d;
			}else if (strcmp(lineHeader, "illum") == 0)
			{
				GLuint illum;
				fscanf(file, "%f\n", &illum);
				materialInfo.illum = illum;
			}

		}

	}

}

void Material::materialStorage(GLuint program)
{
	glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, "material.illum"), materialInfo.illum);

	glProgramUniform1f(program, glGetProgramResourceLocation(program, GL_UNIFORM, "material.shininess"), materialInfo.shininess);
	glProgramUniform1f(program, glGetProgramResourceLocation(program, GL_UNIFORM, "material.opticalDensity"), materialInfo.opticalDensity);
	glProgramUniform1f(program, glGetProgramResourceLocation(program, GL_UNIFORM, "material.alpha"), materialInfo.alpha);

	glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, "material.emissive"), 1, glm::value_ptr(materialInfo.emissive));
	glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, "material.ambient"), 1, glm::value_ptr(materialInfo.ambient));
	glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, "material.diffuse"), 1, glm::value_ptr(materialInfo.diffuse));
	glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, "material.specular"), 1, glm::value_ptr(materialInfo.specular));
	
}