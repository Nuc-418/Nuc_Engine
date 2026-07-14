#version 440 core

// PBR vertex stage for textured models (position + UV + normal). Emits
// world-space position/normal and UV for the shared pbr.frag; vertex colour is
// white so albedo comes from baseColor * the bound albedo texture.

uniform mat4 MVP;
uniform mat4 Model;

in vec3 vPosition;
in vec3 vNormal;
in vec2 vUV;

out vec3 fWorldPos;
out vec3 fWorldNormal;
out vec2 fUV;
out vec3 fColor;

void main()
{
	vec4 world = Model * vec4(vPosition, 1.0);
	fWorldPos = world.xyz;
	fWorldNormal = mat3(transpose(inverse(Model))) * vNormal;
	fUV = vUV;
	fColor = vec3(1.0);
	gl_Position = MVP * vec4(vPosition, 1.0);
}
