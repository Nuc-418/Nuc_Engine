#version 440 core

// Attribute-less fullscreen triangle: three vertices generated from gl_VertexID
// cover the screen, so a post pass needs no vertex buffer — just a bound VAO and
// glDrawArrays(GL_TRIANGLES, 0, 3).

out vec2 vUV;

void main()
{
	vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	vUV = p;
	gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
