#version 440 core

// PBR vertex stage for the built-in primitives (position + normal + per-vertex
// colour). Emits world-space position/normal for the shared pbr.frag; UV is
// unused (no albedo texture) and vertex colour carries the primitive tint.

uniform mat4 MVP;
uniform mat4 Model;

in vec3 vPosition;
in vec3 vNormal;
in vec3 vColor;

out vec3 fWorldPos;
out vec3 fWorldNormal;
out vec2 fUV;
out vec3 fColor;

void main()
{
	vec4 world = Model * vec4(vPosition, 1.0);
	fWorldPos = world.xyz;
	// World-space normal matrix; handles the non-uniform scales primitives use.
	fWorldNormal = mat3(transpose(inverse(Model))) * vNormal;
	fUV = vec2(0.0);
	fColor = vColor;
	gl_Position = MVP * vec4(vPosition, 1.0);
}
