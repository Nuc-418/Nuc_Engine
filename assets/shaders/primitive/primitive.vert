#version 440 core

// Lit shader for the built-in primitive meshes (ground, cube, sphere, ...).
// Takes position + normal + per-vertex colour and passes eye-space position and
// normal to the fragment stage, which shades with the same scene light set the
// textured model shader uses (see Lights::StoreSceneLights). Model/View/MVP and
// the NormalMatrix are supplied by Camera::CamToProgram.

uniform mat4 MVP;
uniform mat4 Model;
uniform mat4 ModelView;
uniform mat3 NormalMatrix;

in vec3 vPosition;
in vec3 vNormal;
in vec3 vColor;

out vec3 vPositionEyeSpace;
out vec3 vNormalEyeSpace;
out vec3 fragPos;   // world space, for the spot-cone test
out vec3 fColor;

void main()
{
	// NormalMatrix = inverse-transpose of the model-view upper 3x3, so normals
	// stay correct under the non-uniform scales the primitives allow.
	vNormalEyeSpace = NormalMatrix * vNormal;
	vPositionEyeSpace = (ModelView * vec4(vPosition, 1.0)).xyz;
	fragPos = (Model * vec4(vPosition, 1.0)).xyz;
	fColor = vColor;
	gl_Position = MVP * vec4(vPosition, 1.0);
}
