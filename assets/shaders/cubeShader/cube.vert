#version 440 core

 uniform mat4 MVP;
 uniform float Time;

 in vec3 vPosition;
 in vec3 vColor;

 out vec3 color;

void main()
{ 
	gl_Position = MVP * vec4(vPosition, 1.0f);
	color = vColor;
}
