#version 440 core

// Procedural HDR sky, evaluated per direction. Renders into the environment
// cubemap so IBL works with no external .hdr asset: a horizon-to-zenith
// gradient, a dim ground, and a bright HDR sun disk for crisp reflections.

in vec3 localPos;
out vec4 FragColor;

void main()
{
	vec3 d = normalize(localPos);

	vec3 horizon = vec3(0.55, 0.66, 0.80);
	vec3 zenith  = vec3(0.12, 0.28, 0.60);
	vec3 ground  = vec3(0.22, 0.20, 0.18);

	float up = clamp(d.y, 0.0, 1.0);
	vec3 sky = mix(horizon, zenith, pow(up, 0.55));
	vec3 col = (d.y < 0.0) ? mix(horizon, ground, clamp(-d.y * 2.5, 0.0, 1.0)) : sky;

	// Bright HDR sun for specular reflections.
	vec3 sunDir = normalize(vec3(0.35, 0.65, 0.45));
	float sun = pow(max(dot(d, sunDir), 0.0), 512.0);
	col += vec3(8.0, 7.4, 6.6) * sun;

	FragColor = vec4(col, 1.0);
}
