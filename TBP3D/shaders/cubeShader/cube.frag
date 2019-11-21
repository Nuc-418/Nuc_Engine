#version 440 core
layout( early_fragment_tests ) in;

 uniform float Time;

 in vec3 color;
 

 out vec4 fColor; // Cor final do fragmento

void main()
{
	fColor = vec4(((cos(Time*5)+1)+0.5f)*color, 1.0f);
	//fColor = vec4(vec3(1,1,1), 1.0f);
}
