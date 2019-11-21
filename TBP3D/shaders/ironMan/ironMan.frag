#version 440 core
layout( early_fragment_tests ) in;

 
 uniform mat4 Model;
 uniform mat4 View;	
 uniform mat3 NormalMatrix;
 uniform float Time;
 uniform sampler2D textureMap;

uniform int nPointLights;
uniform int nDirectionalLights;
uniform int nSpotLights;

/*  - - - - - - - - Struct Uniforms - - - - - - - - -  */
 struct MaterialInfo
{
	int  illum;
	float shininess;
	float opticalDensity;
	float alpha;
	vec3 emissive;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform MaterialInfo material;

struct AmbientLight {
	int switchL;
    vec3 ambient;
};
 uniform AmbientLight ambientLight;


struct DirectionalLight {
	int switchL;
	vec3 direction;		// Dire��o da luz, espa�o do mundo
	vec3 ambient;		// Componente de luz ambiente
	vec3 diffuse;		// Componente de luz difusa
	vec3 specular;		// Componente de luz especular
};
uniform DirectionalLight directionalLight;

struct PointLight {
	int switchL;
	vec3 position;		// Posi��o do ponto de luz, espa�o do mundo

	vec3 ambient;		// Componente de luz ambiente
	vec3 diffuse;		// Componente de luz difusa
	vec3 specular;		// Componente de luz especular

	float constant;		// Coeficiente de atenua��o constante
	float linear;		// Coeficiente de atenua��o linear
	float quadratic;	// Coeficiente de atenua��o quadr�tica
};
uniform PointLight pointLight[50];

struct SpotLight {
	int switchL;
	vec3 position;		
	vec3 direction;
	float cutOff;

	vec3 ambient;		
	vec3 diffuse;		
	vec3 specular;		

	float constant;		
	float linear;		
	float quadratic;	
};
uniform SpotLight spotLight[50];
/*  - - - - - - - - - - - - - - - - - - - - - - - - -  */

 in vec2 uv;
  in vec3 vPositionEyeSpace;
  in vec3 vNormalEyeSpace;
  in vec3 normal;
  in vec3 fragPos;

  out vec4 fColor; // Cor final do fragmento


/*  - - - - - - - Refer�ncia de Fun��es - - - - - - -  */
vec4 calcAmbientLight(AmbientLight light);
vec4 calcDirectionalLight(DirectionalLight light);
vec4 calcPointLight(PointLight light);
vec4 calcSpotLight(SpotLight light);


/*  - - - - - - - - - - - Main - - - - - - - - - - -  */

void main()
{
// C�lculo da componente emissiva do material.
	vec4 emissive = vec4(material.emissive, 1.0);

    vec4 lights;

	lights +=  (calcAmbientLight(ambientLight) * ambientLight.switchL);

	
	lights +=  (calcDirectionalLight(directionalLight)*directionalLight.switchL);
	
	for(int index = 0; index < nPointLights; index++)
		lights += (calcPointLight(pointLight[index])*pointLight[index].switchL);

	for(int index = 0; index < nSpotLights; index++)
		lights += (calcSpotLight(spotLight[index])*spotLight[index].switchL);



	fColor =  (emissive +lights) * texture(textureMap, uv);

}

/*  - - - - - - - - - - Fun��es - - - - - - - - - -  */


vec4 calcAmbientLight(AmbientLight light) {
	// C�lculo da contribui��o da fonte de luz ambiente global, para a cor do objeto.
	vec4 ambient = vec4(material.ambient * light.ambient, 1.0);
	return ambient;
}




vec4 calcDirectionalLight(DirectionalLight light) {
	// C�lculo da reflex�o da componente da luz ambiente.
	vec4 ambient = vec4(material.ambient * light.ambient, 1.0);

	// C�lculo da reflex�o da componente da luz difusa.
	vec3 lightDirectionEyeSpace = (View * vec4(light.direction, 0.0)).xyz;
	vec3 L = normalize(-lightDirectionEyeSpace); // Dire��o inversa � da dire��o luz.
	vec3 N = normalize(vNormalEyeSpace);
	float NdotL = max(dot(N, L), 0.0);
	vec4 diffuse = vec4(material.diffuse * light.diffuse, 1.0) * NdotL;
	
	vec3 V = normalize(-vPositionEyeSpace);
	//vec4 H = normalize(L + V);	// Modelo Blinn-Phong
	vec3 R = reflect(-L, N);
	float RdotV = max(dot(R, V), 0.0);
	//float NdotH = max(dot(N, H), 0.0);	// Modelo Blinn-Phong
	vec4 specular = pow(RdotV, material.shininess) * vec4(light.specular * material.specular, 1.0);

	// C�lculo da contribui��o da fonte de luz direcional para a cor final do fragmento.
	return (ambient + diffuse + specular);
}


vec4 calcPointLight(PointLight light) {
	// C�lculo da reflex�o da componente da luz ambiente.
	vec4 ambient = vec4(material.ambient * light.ambient, 1.0);

	// C�lculo da reflex�o da componente da luz difusa.
	//vec3 lightPositionEyeSpace = mat3(View) * light.position;
	vec3 lightPositionEyeSpace = (View * vec4(light.position, 1.0)).xyz;
	vec3 L = normalize(lightPositionEyeSpace - vPositionEyeSpace);
	vec3 N = normalize(vNormalEyeSpace);
	float NdotL = max(dot(N, L), 0.0);
	vec4 diffuse = vec4(material.diffuse * light.diffuse, 1.0) * NdotL;

	// C�lculo da reflex�o da componente da luz especular.
	// Como os c�lculos est�o a ser realizados nas coordenadas do olho, ent�o a c�mara est� na posi��o (0,0,0).
	// Resulta ent�o um vetor V entre os pontos vPositionEyeSpace e (0,0,0):
	//		V = vPositionEyeSpace - (0,0,0) = (0-vPositionEyeSpace.x, 0-vPositionEyeSpace.y, 0-vPositionEyeSpace.z)
	// Que pode ser simplificado como:
	//		- vPositionEyeSpace
	vec3 V = normalize(-vPositionEyeSpace);
	//vec4 H = normalize(L + V);	// Modelo Blinn-Phong
	vec3 R = reflect(-L, N);
	float RdotV = max(dot(R, V), 0.0);
	//float NdotH = max(dot(N, H), 0.0);	// Modelo Blinn-Phong
	vec4 specular = pow(RdotV, material.shininess) * vec4(light.specular * material.specular, 1.0);
	
	// attenuation
	float dist = length(mat3(View) * light.position - vPositionEyeSpace);	// C�lculo da dist�ncia entre o ponto de luz e o v�rtice
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

	// C�lculo da contribui��o da fonte de luz pontual para a cor final do fragmento.
	return (attenuation * (ambient + diffuse + specular));
}

vec4 calcSpotLight(SpotLight light) {
    vec3 lightDir = normalize(light.position - fragPos);
    float theta = acos(dot(-lightDir,normalize(light.direction)));


	// C�lculo da reflex�o da componente da luz ambiente.
	vec4 ambient = vec4(material.ambient * light.ambient, 1.0);

	

	//{
	// C�lculo da reflex�o da componente da luz difusa.
	//vec3 lightPositionEyeSpace = mat3(View) * light.position;
	vec3 lightPositionEyeSpace = (View * vec4(light.position, 1.0)).xyz;
	vec3 L = normalize(lightPositionEyeSpace - vPositionEyeSpace);
	vec3 N = normalize(vNormalEyeSpace);
	float NdotL = max(dot(N, L), 0.0);
	vec4 diffuse = vec4(material.diffuse * light.diffuse, 1.0) * NdotL;

	// C�lculo da reflex�o da componente da luz especular.
	// Como os c�lculos est�o a ser realizados nas coordenadas do olho, ent�o a c�mara est� na posi��o (0,0,0).
	// Resulta ent�o um vetor V entre os pontos vPositionEyeSpace e (0,0,0):
	//		V = vPositionEyeSpace - (0,0,0) = (0-vPositionEyeSpace.x, 0-vPositionEyeSpace.y, 0-vPositionEyeSpace.z)
	// Que pode ser simplificado como:
	//		- vPositionEyeSpace
	vec3 V = normalize(-vPositionEyeSpace);
	//vec4 H = normalize(L + V);	// Modelo Blinn-Phong
	vec3 R = reflect(-L, N);
	float RdotV = max(dot(R, V), 0.0);
	//float NdotH = max(dot(N, H), 0.0);	// Modelo Blinn-Phong
	vec4 specular = pow(RdotV, material.shininess) * vec4(light.specular * material.specular, 1.0);
	
	// attenuation
	float dist = length(mat3(View) * light.position - vPositionEyeSpace);	// C�lculo da dist�ncia entre o ponto de luz e o v�rtice
	
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

	if(theta > light.cutOff)
	{
		attenuation = 0;
	}

	return (ambient +(attenuation * ( diffuse + specular)));
	//}

}



























































