// Lights: ambient/directional/point/spot light sources and their shader uniforms.

#include "engine/render/Lights.h"
#include <iostream>
#include <sstream>
#include <string> 
#include <glm/gtc/type_ptr.hpp> // value_ptr

#include "engine/render/Shader.h"

using namespace std;
using namespace glm;

void Lights::CheckCacheGeneration()
{
	if (cacheGeneration == Shader::GlobalGeneration())
		return;
	ambientSwitchCache.clear();
	directionalSwitchCache.clear();
	pointSwitchCache.clear();
	spotSwitchCache.clear();
	cacheGeneration = Shader::GlobalGeneration();
}

void Lights::StoreSceneLights(GLuint program)
{
	if (!lightInfo.ambientLight.empty())
		StoreAmbientLights(program);
	else
		glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, "ambientLight.switchL"), 0);

	if (!lightInfo.directionalLight.empty())
		StoreDirectionalLights(program, static_cast<int>(lightInfo.directionalLight.size()));
	else
		glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, "nDirectionalLights"), 0);

	// Point/spot uploads always run: with an empty vector they simply write a
	// count of 0, which stops the shader loops.
	StorePointLights(program, static_cast<int>(lightInfo.pointLight.size()));
	StoreSpotLights(program, static_cast<int>(lightInfo.spotLight.size()));
}

/*Fun��o que cria uma fonte de luz ambiente*/
void Lights::AddAmbientLight(GLuint program, vec3 ambient)
{
	if (lightInfo.ambientLight.size())
		lightInfo.ambientLight.clear();

	AmbientLight ambientLight;
	ambientLight.ambient = ambient;
	lightInfo.ambientLight.push_back(ambientLight);

	StoreAmbientLights(program);

}
/*Fun��o que cria uma fonte de luz direcional*/
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
	StoreDirectionalLights(program, static_cast<int>(lightInfo.directionalLight.size()));

}

/*Fun��o que cria uma fonte de luz pontual*/
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
	StorePointLights(program, static_cast<int>(lightInfo.pointLight.size()));
}

/*Fun��o que cria uma fonte de luz c�nica*/
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
	StoreSpotLights(program, static_cast<int>(lightInfo.spotLight.size()));


}


/*Fun��o que armazena as fontes de luz ambientes*/
void Lights::StoreAmbientLights(GLuint program)
{
	glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, "ambientLight.switchL"), lightInfo.ambientLight[0].switchL);
	glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, "ambientLight.ambient"), 1, glm::value_ptr(lightInfo.ambientLight[0].ambient));
}


/*Fun��o que armazena as fontes de luz direcionais*/
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

/*Fun��o que armazena as fontes de luz pontuais*/
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

/*Fun��o que armazena as fontes de luz c�nicas*/
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


/*Fun��o que armazena os valores dos switches de cada fonte de luz ambiente (ligada/desligada)*/
void Lights::ToggleAmbientLight(GLuint program, bool switchL)
{
	CheckCacheGeneration();
	lightInfo.ambientLight[0].switchL = switchL;

	auto it = ambientSwitchCache.find(program);
	if (it == ambientSwitchCache.end())
		it = ambientSwitchCache.emplace(program, glGetProgramResourceLocation(program, GL_UNIFORM, "ambientLight.switchL")).first;

	glProgramUniform1i(program, it->second, lightInfo.ambientLight[0].switchL);
}


/*Fun��o que armazena os valores dos switches de cada fonte de luz direcional (ligada/desligada)*/
void Lights::ToggleDirectionalLight(GLuint program, bool switchL)
{
	CheckCacheGeneration();
	lightInfo.directionalLight[0].switchL = switchL;

	auto it = directionalSwitchCache.find(program);
	if (it == directionalSwitchCache.end())
		it = directionalSwitchCache.emplace(program, glGetProgramResourceLocation(program, GL_UNIFORM, "directionalLight.switchL")).first;

	glProgramUniform1i(program, it->second, lightInfo.directionalLight[0].switchL);
}

/*Fun��o que armazena os valores dos switches de cada fonte de luz pontual (ligada/desligada)*/
void Lights::TogglePointLight(GLuint program, int lightIndex, bool switchL)
{
	CheckCacheGeneration();
	lightInfo.pointLight[lightIndex].switchL = switchL;

	auto it = pointSwitchCache.find({ program, lightIndex });
	if (it == pointSwitchCache.end()) {
		string swi = "pointLight[" + std::to_string(lightIndex) + "].switchL";
		it = pointSwitchCache.emplace(std::make_pair(program, lightIndex), glGetProgramResourceLocation(program, GL_UNIFORM, swi.data())).first;
	}

	glProgramUniform1i(program, it->second, lightInfo.pointLight[lightIndex].switchL);
}


/*Fun��o que armazena os valores dos switches de cada fonte de luz c�nica (ligada/desligada)*/
void Lights::ToggleSpotLight(GLuint program, int lightIndex, bool switchL)
{
	CheckCacheGeneration();
	lightInfo.spotLight[lightIndex].switchL = switchL;

	auto it = spotSwitchCache.find({ program, lightIndex });
	if (it == spotSwitchCache.end()) {
		string swi = "spotLight[" + std::to_string(lightIndex) + "].switchL";
		it = spotSwitchCache.emplace(std::make_pair(program, lightIndex), glGetProgramResourceLocation(program, GL_UNIFORM, swi.data())).first;
	}

	glProgramUniform1i(program, it->second, lightInfo.spotLight[lightIndex].switchL);
}