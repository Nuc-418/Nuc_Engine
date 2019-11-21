/*
Autores: Francisco Aires (14884)
	- - BASEADO EM : https://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/ - -
Data: 05/06/2019

Descrição: ObjLoader.cpp
Ficheiro capaz de carregar um ficheiro .obj, quer tenha faces triangulares ou quadrangulares
 */

#include "ObjLoader.h"

using namespace std;
bool ObjLoader::LoadObj()
{
	FILE *file;

	file = fopen(filePath, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
		return false;
	}
	else
	{
		std::vector<unsigned int>vertexPosElements, vertexUvElements, vertexNormalElements;
		std::vector<glm::vec3>vertexPos;
		std::vector<glm::vec2>vertexUvs;
		std::vector<glm::vec3>vertexNormals;
		while (!loaded)
		{
			char lineHeader[128];
			// read the first word of the line
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
			{
				loaded = true;

				for (unsigned int indexElement = 0; indexElement < vertexPosElements.size(); indexElement++)
				{
					int indexPos = vertexPosElements[indexElement];
					objInfo.vertexPos.push_back(vertexPos[indexPos - 1]);
				}

				for (unsigned int indexElement = 0; indexElement < vertexUvElements.size(); indexElement++)
				{
					int indexUv = vertexUvElements[indexElement];
					objInfo.vertexUvs.push_back(vertexUvs[indexUv - 1]);
				}

				for (unsigned int indexElement = 0; indexElement < vertexNormalElements.size(); indexElement++)
				{
					int indexNormal = vertexNormalElements[indexElement];
					objInfo.vertexNormals.push_back(vertexNormals[indexNormal - 1]);
				}

				fclose(file);
				return true;
			}


			if (strcmp(lineHeader, "mtllib") == 0)
			{
				fscanf(file, "%s\n",&mtlFile);
			}
			else
			if (strcmp(lineHeader, "v") == 0)
			{
				glm::vec3 vertex;
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				vertexPos.push_back(vertex);
			}
			else
				if (strcmp(lineHeader, "vt") == 0)
				{
					glm::vec3 vertex;
					fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
					vertexUvs.push_back(vertex);
				}
				else
					if (strcmp(lineHeader, "vn") == 0)
					{
						glm::vec3 vertex;
						fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
						vertexNormals.push_back(vertex);
					}
					else
					{
						if (strcmp(lineHeader, "f") == 0)
						{
							std::string vertex1, vertex2, vertex3;
							unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
							int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
							if (matches != 9) {
								cout << "File can't be read : "<< matches << endl;
								return false;
							}
							else
							{
								vertexPosElements.push_back(vertexIndex[0]);
								vertexPosElements.push_back(vertexIndex[1]);
								vertexPosElements.push_back(vertexIndex[2]);
								vertexUvElements.push_back(uvIndex[0]);
								vertexUvElements.push_back(uvIndex[1]);
								vertexUvElements.push_back(uvIndex[2]);
								vertexNormalElements.push_back(normalIndex[0]);
								vertexNormalElements.push_back(normalIndex[1]);
								vertexNormalElements.push_back(normalIndex[2]);
							}
						}
					}
		}


	}
}