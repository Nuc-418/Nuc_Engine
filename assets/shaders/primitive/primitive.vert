#version 440 core

// Lit shader for the built-in primitive meshes (ground, cube, sphere, ...).
// Takes position + normal + per-vertex colour; the fragment stage applies a
// single directional light plus ambient. Model/MVP are supplied by Camera.

uniform mat4 MVP;
uniform mat4 Model;

in vec3 vPosition;
in vec3 vNormal;
in vec3 vColor;

out vec3 fNormal;
out vec3 fColor;

void main()
{
	gl_Position = MVP * vec4(vPosition, 1.0);
	// Uniform scale is assumed for the primitives, so the model's upper 3x3
	// transforms normals correctly enough for shading.
	fNormal = mat3(Model) * vNormal;
	fColor = vColor;
}
