/*
Autores: Francisco Aires (14884)
Data: 05/06/2019

Descrição: Lights.cpp
Ficheiro que cria, envia para o prgrama shader e controla as diferentes fontes de luz
 */


#include"Lights.h"
#include <iostream>
#include <sstream>
#include <string> 
#include <glm/gtc/type_ptr.hpp> // value_ptr

using namespace std;

/*Função que cria uma fonte de luz ambiente*/
void Lights::AddAmbientLight(GLuint program, vec3 ambient)
{
	if (lightInfo.ambientLight.size())
		lightInfo.ambientLight.clear();

	AmbientLight ambientLight;
	ambientLight.ambient = ambient;
	lightInfo.ambientLight.push_back(ambientLight);

	StoreAmbientLights(program);

}
/*Função que cria uma fonte de luz direcional*/
void Lights::AddDirectionalLight(GLuint program, vec3 direction, vec3 ambient, vec3 diffuse, vec3 specular)
{
	if (lightInfo.directionalLight.size())
		lightInfo.directionalLight.clear();

	DirectionalLight directionalLight;
	directionalLight.direction = direction;
	directionalLight.ambient = ambient;
	directionalLight.diffuse = diffuse;
	directionalLight.specular = specular;

	lightInfo.directionalLight.push_back(directionalLight);
	StoreDirectionalLights(program, lightInfo.directionalLight.size());

}

/*Função que cria uma fonte de luz pontual*/
void Lights::AddPointLight(GLuint program, vec3 position, vec3 ambient, vec3 diffuse, vec3 specular, float constant, float linear, float quadratic)
{
	PointLight pointLight;
	pointLight.position = position;
	pointLight.ambient = ambient;
	pointLight.diffuse = diffuse;
	pointLight.specular = specular;
	pointLight.constant = constant;
	pointLight.linear = linear;
	pointLight.quadratic = quadratic;

	lightInfo.pointLight.push_back(pointLight);
	StorePointLights(program, lightInfo.pointLight.size());
}

/*Função que cria uma fonte de luz cónica*/
void Lights::AddSpotLight(GLuint program, vec3 position, vec3 direction, vec3 ambient, vec3 diffuse, vec3 specular, float constant, float linear, float quadratic, float cutOff)
{
	SpotLight spotLight;
	spotLight.position = position;
	spotLight.direction = direction;
	spotLight.cutOff = cutOff;
	spotLight.ambient = ambient;
	spotLight.diffuse = diffuse;
	spotLight.specular = specular;
	spotLight.constant = constant;
	spotLight.linear = linear;
	spotLight.quadratic = quadratic;

	lightInfo.spotLight.push_back(spotLight);
	StoreSpotLights(program, lightInfo.spotLight.size());


}


/*Função que armazena as fontes de luz ambientes*/
void Lights::StoreAmbientLights(GLuint program)
{
	glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, "ambientLight.switchL"), lightInfo.ambientLight[0].switchL);
	glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, "ambientLight.ambient"), 1, glm::value_ptr(lightInfo.ambientLight[0].ambient));
}


/*Função que armazena as fontes de luz direcionais*/
void Lights::StoreDirectionalLights(GLuint program, int vectorSize)
{
	
		string lightType = "directionalLight";

		string swi = lightType  + ".switchL";
		glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, swi.data()), lightInfo.directionalLight[0].switchL);

		string dir = lightType  + ".direction";
		glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, dir.data()), 1, glm::value_ptr(lightInfo.directionalLight[0].direction));

		string amb = lightType  + ".ambient";
		glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, amb.data()), 1, glm::value_ptr(lightInfo.directionalLight[0].ambient));

		string dif = lightType + ".diffuse";
		glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, dif.data()), 1, glm::value_ptr(lightInfo.directionalLight[0].diffuse));

		string spec = lightType + ".specular";
		glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, spec.data()), 1, glm::value_ptr(lightInfo.directionalLight[0].specular));
	

	glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, "nDirectionalLights"), vectorSize);

}

/*Função que armazena as fontes de luz pontuais*/
void Lights::StorePointLights(GLuint program, int vectorSize)
{
	for (int index = 0; index < vectorSize; index++)
	{
		string lightType = "pointLight[";


		string swi = lightType + std::to_string(index) + "].switchL";
		glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, swi.data()), lightInfo.pointLight[index].switchL);

		string pos = lightType + std::to_string(index) + "].position";
		glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, pos.data()), 1, glm::value_ptr(lightInfo.pointLight[index].position));

		string amb = lightType + std::to_string(index) + "].ambient";
		glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, amb.data()), 1, glm::value_ptr(lightInfo.pointLight[index].ambient));

		string dif = lightType + std::to_string(index) + "].diffuse";
		glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, dif.data()), 1, glm::value_ptr(lightInfo.pointLight[index].diffuse));


		string spec = lightType + std::to_string(index) + "].specular";
		glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, spec.data()), 1, glm::value_ptr(lightInfo.pointLight[index].specular));

		string cons = lightType + std::to_string(index) + "].constant";
		glProgramUniform1f(program, glGetProgramResourceLocation(program, GL_UNIFORM, cons.data()), lightInfo.pointLight[index].constant);

		string lin = lightType + std::to_string(index) + "].linear";
		glProgramUniform1f(program, glGetProgramResourceLocation(program, GL_UNIFORM, lin.data()), lightInfo.pointLight[index].linear);

		string quad = lightType + std::to_string(index) + "].quadratic";
		glProgramUniform1f(program, glGetProgramResourceLocation(program, GL_UNIFORM, quad.data()), lightInfo.pointLight[index].quadratic);
	}
	glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, "nPointLights"), vectorSize);

}

/*Função que armazena as fontes de luz cónicas*/
void Lights::StoreSpotLights(GLuint program, int vectorSize)
{
	string lightType = "spotLight[";
	for (int index = 0; index < vectorSize; index++)
	{

		string swi = lightType + std::to_string(index) + "].switchL";
		glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, swi.data()), lightInfo.spotLight[index].switchL);

		string pos = lightType + std::to_string(index) + "].position";
		glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, pos.data()), 1, glm::value_ptr(lightInfo.spotLight[index].position));

		string dir = lightType + std::to_string(index) + "].direction";
		glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, dir.data()), 1, glm::value_ptr(lightInfo.spotLight[index].direction));

		string amb = lightType + std::to_string(index) + "].ambient";
		glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, amb.data()), 1, glm::value_ptr(lightInfo.spotLight[index].ambient));

		string dif = lightType + std::to_string(index) + "].diffuse";
		glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, dif.data()), 1, glm::value_ptr(lightInfo.spotLight[index].diffuse));

		string spec = lightType + std::to_string(index) + "].specular";
		glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, spec.data()), 1, glm::value_ptr(lightInfo.spotLight[index].specular));

		string cons = lightType + std::to_string(index) + "].constant";
		glProgramUniform1f(program, glGetProgramResourceLocation(program, GL_UNIFORM, cons.data()), lightInfo.spotLight[index].constant);

		string lin = lightType + std::to_string(index) + "].linear";
		glProgramUniform1f(program, glGetProgramResourceLocation(program, GL_UNIFORM, lin.data()), lightInfo.spotLight[index].linear);

		string quad = lightType + std::to_string(index) + "].quadratic";
		glProgramUniform1f(program, glGetProgramResourceLocation(program, GL_UNIFORM, quad.data()), lightInfo.spotLight[index].quadratic);

		string cut = lightType + std::to_string(index) + "].cutOff";
		glProgramUniform1f(program, glGetProgramResourceLocation(program, GL_UNIFORM, cut.data()), lightInfo.spotLight[index].cutOff);

	}

	glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, "nSpotLights"), vectorSize);
}


/*Função que armazena os valores dos switches de cada fonte de luz ambiente (ligada/desligada)*/
void Lights::ToggleAmbientLight(GLuint program, bool switchL)
{
	lightInfo.ambientLight[0].switchL = switchL;
	glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, "ambientLight.switchL"), lightInfo.ambientLight[0].switchL);
}


/*Função que armazena os valores dos switches de cada fonte de luz direcional (ligada/desligada)*/
void Lights::ToggleDirectionalLight(GLuint program, bool switchL)
{
	lightInfo.directionalLight[0].switchL = switchL;
	glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, "directionalLight.switchL"), lightInfo.directionalLight[0].switchL);
	
}

/*Função que armazena os valores dos switches de cada fonte de luz pontual (ligada/desligada)*/
void Lights::TogglePointLight(GLuint program, int lightIndex, bool switchL)
{
	lightInfo.pointLight[lightIndex].switchL = switchL;

	string lightType = "pointLight[";
	string swi = lightType + std::to_string(lightIndex) + "].switchL";
	glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, swi.data()), lightInfo.pointLight[lightIndex].switchL);
}


/*Função que armazena os valores dos switches de cada fonte de luz cónica (ligada/desligada)*/
void Lights::ToggleSpotLight(GLuint program, int lightIndex, bool switchL)
{
	lightInfo.spotLight[lightIndex].switchL = switchL;

	string lightType = "spotLight[";
	string swi = lightType + std::to_string(lightIndex) + "].switchL";
	glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, swi.data()), lightInfo.spotLight[lightIndex].switchL);

}