/*
Autores: Francisco Aires (14884)
Data: 05/06/2019

Descri��o: Mesh.cpp
Ficheiro que trata dados da mesh de um dado objeto
 */

#include "Mesh.h"

 /*Fun��o que associa um array de posi��es e um array de cores � mesh*/
void Mesh::AssignPosColor(vector<glm::vec3>* positionArray, vector<glm::vec3>*  colorArray)
{
	//N�mero de v�rtices
	nVertex = positionArray->size();
	//Array de posi��es
	vertexArrayPtr[Positions] = positionArray;
	//Array de cores
	vertexArrayPtr[Colors] = colorArray;

	//Cria-se a mesh
	Load();
}

/*Fun��o que associa um array de posi��es, um array de normais e um array de coordenadas de textura � mesh*/
void Mesh::AssignPosUvNorm(vector<glm::vec3>* positionArray, vector<glm::vec2>* uvArray, vector<glm::vec3>* normalArray)
{
	//N�mero de v�rtices
	nVertex = positionArray->size();
	//Array de posi��es
	vertexArrayPtr[Positions] = positionArray;
	//Array de normais
	vertexArrayPtr[Normals] = normalArray;
	//Array de coordenadas de textura
	vertexUvArrayPtr = uvArray;

	//Cria-se a mesh
	Load();
}

/*Fun��o que associa um array de �ndices � mesh*/
void Mesh::AssignElementArray(vector<GLuint>* elementArray)
{
	nElements = elementArray->size();
	elementArrayPtr = elementArray;

	//Carrega a mesh
	Load();
}

/*Fun��o que associa um novo array de posi��es � mesh*/
void Mesh::RewriteVertexPos(vector<glm::vec3>* positionArray)
{
	//Array de posi��es
	vertexArrayPtr[Positions] = positionArray;
	//Vetor que armazena o primeiro �ndice do array de posi��es da mesh
	glm::vec3* auxVecStartPtr = &vertexArrayPtr[Positions]->at(0);
	//� criado e vinculado um novo VBO 
	glBindBuffer(GL_ARRAY_BUFFER, VBO[Positions]);
	//Passa-se a nova informa��o sobre posi��es de v�rtice para o VBO criado
	glBufferData(GL_ARRAY_BUFFER, vertexArrayPtr[Positions]->size() * sizeof(glm::vec3), auxVecStartPtr, GL_STATIC_DRAW);

	//Cria-se a mesh
	Load();
}

/*Cria-se um VAO*/
void Mesh::LoadVAO()
{
	if (VAO == 0) //se n�o houver um VAO vinculado
	{
		//Geram-se nomes para um VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		//Cria-se e vincula-se um VAO
		cout << "VAO : " << VAO << endl;
	}
}
/*Criam-se VBOs*/
void Mesh::LoadVBOs()
{
	for (int index = 0; index < 3; index++)
	{
		//Se existir um v�rtice
		if (vertexArrayPtr[index])
		{
			
			//Cria-se um apontador que aponta para o primeiro elemento desse v�rtice
			glm::vec3* auxVecStartPtr = &vertexArrayPtr[index]->at(0);
			//Criam-se nomes para o VBO
			glGenBuffers(1, &VBO[index]);
			//Cria-se e vincula-se o VBO
			glBindBuffer(GL_ARRAY_BUFFER, VBO[index]);
			//Passa-se a informa��o do CPU para o VBO 
			glBufferData(GL_ARRAY_BUFFER, (vertexArrayPtr[index]->size() * sizeof(glm::vec3)), auxVecStartPtr, GL_STATIC_DRAW);

			cout << "Array of " << resourceLocations[index] << " loaded to VBO : " << VBO[index] << endl;

			//Encontra-se a localiza��o do objeto no programa
			GLuint id = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, resourceLocations[index]);
			//Une-se o VBO ao VAO e liga-se o VAO ao programa shader
			glVertexAttribPointer(id, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glEnableVertexAttribArray(id);

			cout << "VBO " << resourceLocations[index] << " : " << VBO[index] << " binded " << endl;
		}
	}
	if (vertexUvArrayPtr)
	{
		//Cria-se um apontador que aponta para o primeiro elemento dessa coordenada
		glm::vec2* auxVecStartPtr = &vertexUvArrayPtr->at(0);
		glGenBuffers(1, &VBO[UVs]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[UVs]);
		glBufferData(GL_ARRAY_BUFFER, (vertexUvArrayPtr->size() * sizeof(glm::vec2)), auxVecStartPtr, GL_STATIC_DRAW);

		cout << "Array of " << resourceLocations[UVs] << " loaded to VBO : " << VBO[UVs] << endl;

		GLuint id = glGetProgramResourceLocation(program, GL_PROGRAM_INPUT, resourceLocations[UVs]);
		glVertexAttribPointer(id, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(id);

	}

}
/*Cria-se o EBO*/
void Mesh::LoadEBO()
{
	if (elementArrayPtr != NULL)
	{
		GLuint* aux = &elementArrayPtr->at(0);
		glGenBuffers(1, &EBO);
		cout << "EBO : " << EBO << endl;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementArrayPtr->size() * sizeof(GLuint),aux, GL_STATIC_DRAW);
		cout << "Array of elements loaded to VAO : " << EBO << endl;
	}


}


void Mesh::Load()
{
	LoadVAO();
	LoadVBOs();
	LoadEBO();

	for (int index = 0; index < 4; index++)
		vertexArrayPtr[index] = NULL;
	elementArrayPtr = NULL;
}

