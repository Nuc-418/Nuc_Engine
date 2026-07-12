#version 440 core

 uniform mat4 MVP;
 uniform mat4 Model;
 uniform mat4 View;	
 uniform mat4 ModelView;
 uniform mat4 Projection;
 uniform mat3 NormalMatrix;
 uniform int offsetToggle;
 uniform float Time;

in vec3 vPosition;
in vec3 vNormal;
in vec2 vUV;

out vec3 vPositionEyeSpace;
out vec3 vNormalEyeSpace;
out vec3 normal;
out vec3 fragPos;
out vec2 uv;

vec3 Offset();

void main()
{ 



	// Transformar a normal do vértice.
	vNormalEyeSpace = vec3(ModelView * vec4(vNormal, 0.0f));

	uv = vUV;

	
	vec3 offset;
	offset = (Offset()*offsetToggle);

	// Posição do vértice em coordenadas do olho.
	vPositionEyeSpace = (ModelView * vec4(vPosition + offset, 1.0)).xyz;

	fragPos = vec3(Model * vec4(vPosition + offset,1.0));

	gl_Position = MVP *  vec4(vPosition+offset , 1.0f);
}

vec3 Offset()
{
vec3 offset;

offset = vec3((cos((normalize(Time)*1*vPosition.y+Time*2)*0.5)),(cos((normalize(Time)*0.5*vPosition.x+Time*2))*1),(cos((normalize(Time)*1*vPosition.y+Time*2))*0.5));

return offset;
}
