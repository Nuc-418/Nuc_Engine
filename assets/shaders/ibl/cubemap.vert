#version 440 core

// Shared vertex stage for the IBL capture passes: renders a unit cube through a
// 90-degree capture projection/view and passes the local position (a direction
// from the cube center) to the fragment stage.

layout(location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 localPos;

void main()
{
	localPos = aPos;
	gl_Position = projection * view * vec4(aPos, 1.0);
}
