// Material: .mtl parsing and upload of the material.* shader uniforms.

#pragma once
#include <string>
#include <GL/glew.h>

#include <glm/glm.hpp> // vec3, vec4, ivec4, mat4, ...

struct MaterialInfo
{
	GLuint  illum;
	GLfloat shininess;
	GLfloat opticalDensity;
	GLfloat alpha;
	glm::vec3 emissive;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

class Material
{
public:
	MaterialInfo materialInfo = {};

	void materialStorage(GLuint program);

	void loadMaterial(char* path);

	bool loaded = false;
};







