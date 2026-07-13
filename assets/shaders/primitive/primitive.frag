#version 440 core

// Directional light + ambient shading for the primitive meshes. The light is
// uploaded as plain uniforms (see Lights::StorePrimitiveLight) rather than the
// array-based light blocks the textured model shader uses.

uniform vec3 uLightDir;    // direction the light travels, world space
uniform vec3 uLightColor;  // directional diffuse colour
uniform vec3 uAmbient;     // ambient colour
uniform int  uLightOn;     // directional light on/off

in vec3 fNormal;
in vec3 fColor;

out vec4 outColor;

void main()
{
	vec3 N = normalize(fNormal);
	vec3 L = normalize(-uLightDir);
	float diffuse = max(dot(N, L), 0.0);
	vec3 lighting = uAmbient + uLightColor * diffuse * float(uLightOn);
	outColor = vec4(fColor * lighting, 1.0);
}
