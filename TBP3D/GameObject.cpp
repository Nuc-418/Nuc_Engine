#include <iostream>
#include "GameObject.h"
bool GameObject::LoadObjFile(GLuint programShader, string folderPath, string fileName)
{

	string objPath = folderPath  + fileName;
	
	ObjLoader objLoader((char*)objPath.data());
	if (objLoader.loaded)
	{
		meshRenderer.SetProgramShader(programShader);
		meshRenderer.mesh.AssignPosUvNorm(&objLoader.objInfo.vertexPos, &objLoader.objInfo.vertexUvs, &objLoader.objInfo.vertexNormals);
		cout << "Loaded : " << objPath << endl;

		string mtlPath = folderPath  + objLoader.mtlFile.data();
		material.loadMaterial((char*)mtlPath.data());
		if (material.loaded)
		{
			material.materialStorage(meshRenderer.program);
			cout << "Loaded : " << mtlPath << endl;
			return true;
		}
		cout << "Error Loading : " << mtlPath << endl;

		return false;
	}

	cout << "Error Loading : " << objPath << endl;

	return false;
}

void GameObject::CreateObjPosColor(GLuint programShader, vector<glm::vec3>* positionArray, vector<glm::vec3>*  colorArray)
{
	meshRenderer.SetProgramShader(programShader);
	meshRenderer.mesh.AssignPosColor(positionArray, colorArray);
}

void GameObject::CreateObjPosUvNorm(GLuint programShader, vector<glm::vec3>* positionArray, vector<glm::vec2>* uvArray, vector<glm::vec3>* normalArray)
{
	meshRenderer.SetProgramShader(programShader);
	meshRenderer.mesh.AssignPosUvNorm(positionArray, uvArray, normalArray);
}







