// MeshRenderer: draws a Mesh with a shader program and a Transform.

#include "engine/render/MeshRenderer.h"
#include <iostream>

using namespace std;

/*Atribui-se um programa shader*/
void MeshRenderer::SetProgramShader(GLuint shaderProgram)
{
	program = shaderProgram;
	mesh.program = program;
	cout << "Program Shader : " << "  " << program << endl;
}


/*Faz-se o desenho das primitivas*/
void MeshRenderer::Draw(GLenum mode,Camera* camera)
{
	transformPtr->UpdateModel();

	glUseProgram(program);
	glBindVertexArray(mesh.VAO);

	camera->CamToProgram(program, transformPtr->model);

	if (mesh.EBO == 0)
		glDrawArrays(mode, 0, mesh.nVertex);
	else
	{
		//Vincula-se o buffer de índices ao EBO 
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
		//Desenham-se as primitivas com recurso aos índices
		glDrawElements(mode, mesh.nElements, GL_UNSIGNED_INT, 0);
	}
		

}

