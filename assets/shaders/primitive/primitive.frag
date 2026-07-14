#version 440 core

// Lit shading for the built-in primitive meshes. Uses the SAME scene light
// uniform blocks as the textured model shader (ambient + one directional + up
// to 50 point + 50 spot lights), so every LightComponent in the world lights
// the primitives too. Per-vertex colour is the surface albedo; a fixed
// shininess stands in for the material the models carry.

uniform mat4 View;

uniform int nPointLights;
uniform int nDirectionalLights;
uniform int nSpotLights;

struct AmbientLight {
	int switchL;
	vec3 ambient;
};
uniform AmbientLight ambientLight;

struct DirectionalLight {
	int switchL;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirectionalLight directionalLight;

struct PointLight {
	int switchL;
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;
	float linear;
	float quadratic;
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

in vec3 vPositionEyeSpace;
in vec3 vNormalEyeSpace;
in vec3 fragPos;
in vec3 fColor;

out vec4 outColor;

const float kShininess = 32.0;
const float kSpecStrength = 0.3; // primitives have no material; keep highlights modest

vec3 calcDirectional(DirectionalLight light)
{
	vec3 ambient = fColor * light.ambient;

	vec3 lightDirEye = (View * vec4(light.direction, 0.0)).xyz;
	vec3 L = normalize(-lightDirEye);
	vec3 N = normalize(vNormalEyeSpace);
	float NdotL = max(dot(N, L), 0.0);
	vec3 diffuse = fColor * light.diffuse * NdotL;

	vec3 V = normalize(-vPositionEyeSpace);
	vec3 R = reflect(-L, N);
	float spec = pow(max(dot(R, V), 0.0), kShininess);
	vec3 specular = light.specular * spec * kSpecStrength;

	return ambient + diffuse + specular;
}

vec3 calcPoint(PointLight light)
{
	vec3 ambient = fColor * light.ambient;

	vec3 lightPosEye = (View * vec4(light.position, 1.0)).xyz;
	vec3 L = normalize(lightPosEye - vPositionEyeSpace);
	vec3 N = normalize(vNormalEyeSpace);
	float NdotL = max(dot(N, L), 0.0);
	vec3 diffuse = fColor * light.diffuse * NdotL;

	vec3 V = normalize(-vPositionEyeSpace);
	vec3 R = reflect(-L, N);
	float spec = pow(max(dot(R, V), 0.0), kShininess);
	vec3 specular = light.specular * spec * kSpecStrength;

	float dist = length(lightPosEye - vPositionEyeSpace);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

	return attenuation * (ambient + diffuse + specular);
}

vec3 calcSpot(SpotLight light)
{
	vec3 ambient = fColor * light.ambient;

	// Cone test in world space (matches the model shader).
	vec3 lightDir = normalize(light.position - fragPos);
	float theta = acos(dot(-lightDir, normalize(light.direction)));

	vec3 lightPosEye = (View * vec4(light.position, 1.0)).xyz;
	vec3 L = normalize(lightPosEye - vPositionEyeSpace);
	vec3 N = normalize(vNormalEyeSpace);
	float NdotL = max(dot(N, L), 0.0);
	vec3 diffuse = fColor * light.diffuse * NdotL;

	vec3 V = normalize(-vPositionEyeSpace);
	vec3 R = reflect(-L, N);
	float spec = pow(max(dot(R, V), 0.0), kShininess);
	vec3 specular = light.specular * spec * kSpecStrength;

	float dist = length(lightPosEye - vPositionEyeSpace);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
	if (theta > light.cutOff)
		attenuation = 0.0;

	return ambient + attenuation * (diffuse + specular);
}

void main()
{
	vec3 lighting = fColor * ambientLight.ambient * float(ambientLight.switchL);

	if (nDirectionalLights > 0)
		lighting += calcDirectional(directionalLight) * float(directionalLight.switchL);

	for (int i = 0; i < nPointLights; i++)
		lighting += calcPoint(pointLight[i]) * float(pointLight[i].switchL);

	for (int i = 0; i < nSpotLights; i++)
		lighting += calcSpot(spotLight[i]) * float(spotLight[i].switchL);

	outColor = vec4(lighting, 1.0);
}
