#version 440 core

// Physically based shading (Cook-Torrance, metallic/roughness workflow), in
// WORLD space. Lit by the scene's directional/point/spot lights — the SAME
// uniform blocks the light-upload path (Lights::StoreSceneLights) fills — plus
// an ambient term: image-based lighting when an environment is bound
// (uHasIBL == 1), otherwise a flat ambient fallback (the world environment
// term). Shared by the primitive and model programs (different vertex shaders,
// identical fragment interface).

uniform vec3 CamPos; // camera world position (Camera::CamToProgram)

// ---- Material (parameters; albedo optionally textured for models) ----
struct PbrMaterial {
	vec3  baseColor;
	float metallic;
	float roughness;
	vec3  emissive;
	float ao;
};
uniform PbrMaterial pbrMaterial;
uniform int       uHasAlbedoTex; // models sample textureMap; primitives use vertex colour
uniform sampler2D textureMap;

// ---- Lights (shared upload path) ----
uniform int nPointLights;
uniform int nDirectionalLights;
uniform int nSpotLights;

struct AmbientLight { int switchL; vec3 ambient; };
uniform AmbientLight ambientLight;

struct DirectionalLight { int switchL; vec3 direction; vec3 ambient; vec3 diffuse; vec3 specular; };
uniform DirectionalLight directionalLight;

struct PointLight {
	int switchL; vec3 position; vec3 ambient; vec3 diffuse; vec3 specular;
	float constant; float linear; float quadratic;
};
uniform PointLight pointLight[50];

struct SpotLight {
	int switchL; vec3 position; vec3 direction; float cutOff;
	vec3 ambient; vec3 diffuse; vec3 specular;
	float constant; float linear; float quadratic;
};
uniform SpotLight spotLight[50];

// ---- Image-based lighting ----
uniform int         uHasIBL;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D   brdfLUT;
uniform float       uMaxReflectionLod;

in vec3 fWorldPos;
in vec3 fWorldNormal;
in vec2 fUV;
in vec3 fColor;

out vec4 outColor;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	return a2 / (PI * denom * denom + 1e-7);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;
	return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness)
	     * GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 CookTorrance(vec3 N, vec3 V, vec3 L, vec3 radiance, vec3 albedo,
                  float metallic, float roughness, vec3 F0)
{
	vec3 H = normalize(V + L);
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 numerator = NDF * G * F;
	float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 1e-4;
	vec3 specular = numerator / denom;

	vec3 kS = F;
	vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
	float NdotL = max(dot(N, L), 0.0);
	return (kD * albedo / PI + specular) * radiance * NdotL;
}

void main()
{
	vec3 albedo = pbrMaterial.baseColor * fColor;
	if (uHasAlbedoTex == 1)
		albedo *= texture(textureMap, fUV).rgb;
	float metallic = clamp(pbrMaterial.metallic, 0.0, 1.0);
	float roughness = clamp(pbrMaterial.roughness, 0.04, 1.0);
	float ao = pbrMaterial.ao;

	vec3 N = normalize(fWorldNormal);
	vec3 V = normalize(CamPos - fWorldPos);
	vec3 F0 = mix(vec3(0.04), albedo, metallic);

	vec3 Lo = vec3(0.0);

	if (nDirectionalLights > 0 && directionalLight.switchL != 0) {
		vec3 L = normalize(-directionalLight.direction);
		Lo += CookTorrance(N, V, L, directionalLight.diffuse, albedo, metallic, roughness, F0);
	}

	for (int i = 0; i < nPointLights; i++) {
		if (pointLight[i].switchL == 0) continue;
		vec3 d = pointLight[i].position - fWorldPos;
		float dist = length(d);
		vec3 L = d / max(dist, 1e-4);
		float att = 1.0 / (pointLight[i].constant + pointLight[i].linear * dist + pointLight[i].quadratic * dist * dist);
		Lo += CookTorrance(N, V, L, pointLight[i].diffuse * att, albedo, metallic, roughness, F0);
	}

	for (int i = 0; i < nSpotLights; i++) {
		if (spotLight[i].switchL == 0) continue;
		vec3 d = spotLight[i].position - fWorldPos;
		float dist = length(d);
		vec3 L = d / max(dist, 1e-4);
		float theta = acos(dot(-L, normalize(spotLight[i].direction)));
		float att = 1.0 / (spotLight[i].constant + spotLight[i].linear * dist + spotLight[i].quadratic * dist * dist);
		if (theta > spotLight[i].cutOff) att = 0.0;
		Lo += CookTorrance(N, V, L, spotLight[i].diffuse * att, albedo, metallic, roughness, F0);
	}

	// Ambient (IBL or flat) is the environment light: gate it on the ambient
	// switch so turning off the Environment ambient — together with the Light
	// actors — makes the scene go dark (only emissive remains). No "soft fill"
	// fallback: off means off.
	vec3 ambient = vec3(0.0);
	if (ambientLight.switchL != 0) {
		if (uHasIBL == 1) {
			vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
			vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
			vec3 irradiance = texture(irradianceMap, N).rgb;
			vec3 diffuseIBL = irradiance * albedo;
			vec3 R = reflect(-V, N);
			vec3 prefiltered = textureLod(prefilterMap, R, roughness * uMaxReflectionLod).rgb;
			vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
			vec3 specularIBL = prefiltered * (F * brdf.x + brdf.y);
			ambient = (kD * diffuseIBL + specularIBL) * ao;
		} else {
			ambient = ambientLight.ambient * albedo * ao;
		}
	}

	// Linear HDR out; the post pass (PostProcess::Tonemap) does tonemapping + gamma.
	vec3 color = ambient + Lo + pbrMaterial.emissive;
	outColor = vec4(color, 1.0);
}
