#version 440 core

// Tonemapping + gamma: maps the linear-HDR scene texture down to a displayable
// LDR image. ACES filmic approximation followed by sRGB gamma.

in vec2 vUV;

uniform sampler2D hdrBuffer;
uniform float exposure;

out vec4 outColor;

vec3 ACESFilm(vec3 x)
{
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
	vec3 hdr = texture(hdrBuffer, vUV).rgb * exposure;
	vec3 mapped = ACESFilm(hdr);
	mapped = pow(mapped, vec3(1.0 / 2.2));
	outColor = vec4(mapped, 1.0);
}
