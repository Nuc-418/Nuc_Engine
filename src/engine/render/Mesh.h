// Mesh: owns the VAO/VBOs/EBO for a set of vertex arrays.

#pragma once
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp> // vec3, vec4, ivec4, mat4, ...
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include <GLFW/glfw3.h>
#include <vector>

class Mesh
{

public:

	// Vertex attribute slots (indices into vertexArrayPtr / VBO).
	// An enum, not macros: identifiers like "Colors" also appear as members
	// of third-party types (ImGuiStyle), which macros would break.
	enum VertexAttribute { Positions = 0, Normals = 1, Colors = 2, UVs = 3 };

	//Vertex info
	//OpenGL Var //1-Pos,2-Norm,3-Color,4-UVs
	int nVertex = 0;
	std::vector<glm::vec3>* vertexArrayPtr[3] = { NULL };//Ver
	std::vector<glm::vec2>* vertexUvArrayPtr = NULL;


	//Element info
	int nElements = 0;
	std::vector<GLuint>* elementArrayPtr = { NULL };



	GLuint program = 0;
	GLuint VAO = 0;
	GLuint VBO[4] = { 0 };
	GLuint EBO = { 0 };
	const GLchar *resourceLocations[4] = { "vPosition","vNormal","vColor","vUV" };


	void AssignPosColor(std::vector<glm::vec3>* positionArray, std::vector<glm::vec3>*  colorArray);
	void AssignPosNormColor(std::vector<glm::vec3>* positionArray, std::vector<glm::vec3>* normalArray, std::vector<glm::vec3>* colorArray);
	void AssignPosUvNorm(std::vector<glm::vec3>* positionArray, std::vector<glm::vec2>* uvArray, std::vector<glm::vec3>* normalArray);
	void AssignElementArray(std::vector<GLuint>* elementArray);

	void RewriteVertexPos(std::vector<glm::vec3>* positionArray);

	// Deletes the VAO/VBOs/EBO. Must run while the GL context is alive.
	void Unload();

	// Local-space bounds captured when vertex positions are uploaded
	// (the CPU arrays are released right after upload); used for picking.
	glm::vec3 aabbMin = glm::vec3(0.0f);
	glm::vec3 aabbMax = glm::vec3(0.0f);
	bool hasAabb = false;

private:
	void CaptureBounds(std::vector<glm::vec3>* positionArray);

	void LoadVAO();
	void LoadVBOs();
	void LoadEBO();

	void Load();
};