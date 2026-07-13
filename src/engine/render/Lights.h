// Lights: ambient/directional/point/spot light sources and their shader uniforms.

#pragma once
#include <map>
#include <utility>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

using namespace glm;
using namespace std;

/*Fonte de luz ambiente*/
struct AmbientLight {

	//Estado da luz - On/Off
	int switchL= true;

	//Componente de luz ambiente global
	vec3 ambient;	

};

/*Fonte de luz direcional*/
struct DirectionalLight {

	//Estado da luz - On/Off
	int switchL = true;

	//Dire��o da luz no espa�o do mundo
	vec3 direction;		

	//Componente de luz ambiente
	vec3 ambient;	

	//Componente de luz difusa
	vec3 diffuse;

	//Componente de luz especular
	vec3 specular;		
};

/*Fonte de luz pontual*/
struct PointLight {

	//Estado da luz - On/Off
	int switchL = true;

	//Posi��o do ponto de luz no espa�o do mundo
	vec3 position;		

	//Componente de luz ambiente
	vec3 ambient;	

	//Componente de luz difusa
	vec3 diffuse;	

	//Componente de luz especular
	vec3 specular;		

	//Coeficiente de atenua��o constante
	float constant;	

	//Coeficiente de atenua��o linear
	float linear;		

	//Coeficiente de atenua��o quadr�tica
	float quadratic;	
};

/*Fonte de luz c�nica*/
struct SpotLight {

	//Estado da luz - On/Off
	int switchL = true;

	//Posi��o do ponto de luz no espa�o do mundo
	vec3 position;		
	vec3 direction;
	float cutOff;

	//Componente de luz ambiente
	vec3 ambient;	

	//Componente de luz difusa
	vec3 diffuse;	

	//Componente de luz especular
	vec3 specular;		

	//Coeficiente de atenua��o constante
	float constant;		

	//Coeficiente de atenua��o linear
	float linear;	

	//Coeficiente de atenua��o quadr�tica
	float quadratic;	
};

/*Defini��o de vetores que armazenam v�rias fontes de luz de um determinado tipo*/
struct VectorLight
{
	vector<AmbientLight> ambientLight;
	vector<DirectionalLight> directionalLight;
	vector<PointLight> pointLight;
	vector<SpotLight> spotLight;

};

class Lights
{
public:
	//Cria-se um vetor com informa��es sobre cada luz
	VectorLight lightInfo;


	void AddAmbientLight(GLuint program,vec3 ambient);
	void AddDirectionalLight(GLuint program, vec3 direction,vec3 ambient, vec3 diffuse, vec3 specular);
	void AddPointLight(GLuint program, vec3 position,vec3 ambient,vec3 diffuse,vec3 specular,float constant,float linear,float quadratic);
	void AddSpotLight(GLuint program, vec3 position,vec3 direction,vec3 ambient,vec3 diffuse,vec3 specular,float constant,float linear,float quadratic, float cutOff);

	void ToggleAmbientLight(GLuint program,bool switchL);
	void ToggleDirectionalLight(GLuint program, bool switchL);
	void TogglePointLight(GLuint program, int lightIndex, bool switchL);
	void ToggleSpotLight(GLuint program, int lightIndex, bool switchL);

	// Uploads the first directional + ambient light as the plain uLight*
	// uniforms used by the primitive shader (see primitive.frag). Safe to call
	// with no lights present (falls back to a lit-from-above default).
	void StorePrimitiveLight(GLuint program);

	void StoreAmbientLights(GLuint program);
	void StoreDirectionalLights(GLuint program,int vectorSize);
	void StorePointLights(GLuint program, int vectorSize);
	void StoreSpotLights(GLuint program, int vectorSize);

private:
	// switchL uniform locations cached for the per-frame Toggle* calls,
	// keyed by program (plus light index for point/spot lights).
	// Programs are created once at load time and never recreated.
	std::map<GLuint, GLint> ambientSwitchCache;
	std::map<GLuint, GLint> directionalSwitchCache;
	std::map<std::pair<GLuint, int>, GLint> pointSwitchCache;
	std::map<std::pair<GLuint, int>, GLint> spotSwitchCache;
};



