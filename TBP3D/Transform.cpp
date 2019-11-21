#include "Transform.h"

//Transform Func/////////////////////
void Transform::Rotate(glm::vec3 deltaRotationVec)
{
	glm::vec3 offset = { -1.0f,1.0f,1.0f };
	rotation += deltaRotationVec * offset ;

	
}

void Transform::SetRotation(glm::vec3 rotationVec)
{
	rotation = rotationVec;
}

void Transform::Translate(glm::vec3 deltaPosVec)
{
	position += deltaPosVec;
}

void Transform::SetPos(glm::vec3 posVec)
{
	position = posVec;
}

void Transform::CalcTranslationMatrix()
{
	//Translation Matrix
	translationMatrix = glm::translate(glm::mat4(), position);
}

void Transform::CalcRotationMatrix()
{
	//Ang Otimization
	if (rotation.x >= (2 * SMALL_PI) || rotation.x <= -(2 * SMALL_PI))
		rotation.x = 0;
	if (rotation.y >= (2 * SMALL_PI) || rotation.y <= -(2 * SMALL_PI))
		rotation.y = 0;
	if (rotation.z >= (2 * SMALL_PI) || rotation.z <= -(2 * SMALL_PI))
		rotation.z = 0;

	//Rotation Matrix
	rotationMatrix = (
		glm::rotate(glm::mat4(1.0f), (GLfloat)(rotation.x), glm::vec3(0.0f, 1.0f, 0.0f))//yaw
		*glm::rotate(glm::mat4(1.0f), (GLfloat)(rotation.y), glm::vec3(1.0f, 0.0f, 0.0f))//pich;
		*glm::rotate(glm::mat4(1.0f), (GLfloat)(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f))//roll;
		);
}

void Transform::CalcScaleMatrix()
{
	scaleMatrix =  glm::scale(glm::mat4(1.0f),scale);
}

void Transform::CalcLocalAxis()
{
	forward = model * defaultForward;
	up = model * defaultUp;
	right = -(model * defaultRight);
}

void Transform::CalcLocalAxisCam()
{
	forward = rotationMatrix * defaultForward;
	up = rotationMatrix * defaultUp;
	right =- rotationMatrix * defaultRight;
}

void Transform::CalcModel()
{
	model = translationMatrix * rotationMatrix * scaleMatrix;
}

void Transform::UpdateModel()
{

	CalcScaleMatrix();

	CalcRotationMatrix();

	CalcTranslationMatrix();

	CalcModel();

	CalcLocalAxis();


}



