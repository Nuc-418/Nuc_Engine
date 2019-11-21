/*
Autores: Francisco Aires
Data: 05/06/2019
Email : rruunneeftw@gmail.com

Descrição: Source.cpp
Ficheiro capaz de unir a informação dos restantes ficheiros .cpp e .h, criando um ambiente navegável
 */


#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")

 /*Largura e altura da janela de visualização*/
#define WIDTH 800
#define HEIGHT 600

#include <iostream>
#include <vector>
#include "Texture.h"
#include "Time.h"
#include "GameObject.h"
#include "Camera.h"
#include "UserInputs.h"
#include "Controller.h"
#include "LoadShaders.h"
#include "Lights.h"
#include "Window.h"
#include "math.h"
#include <time.h> 

/* Funções que, conjuntamente, carregam o programa shader e os objetos, atualizam o mundo e permitem desenhar primitivas */
#pragma region Func
void loadProgramShader(void);
void LoadObjects(void);
void Update(void);
void Draw(void);
#pragma endregion

#pragma region Var
Window window;
Lights lights;
GLenum renderMode = GL_TRIANGLES;
UserInputs gameUserInputs;
Controller controller;
int offsetToggle;

/*Programa shader, "GameObject" e textura dos objetos Iron Man*/
//Iron Man
GLuint ironManProgramShader = 0;
GameObject ironMan;
GameObject ironMan2;
Texture ironManTexture;

/*Programa shader e "GameObject" do cubo*/
GLuint cubeProgramShader = 0;
GameObject cubes[100];
GameObject cube;
GameObject cube2;

/*Parâmetros relativos à câmara, à posição, alvo e vetor normal*/
glm::vec3 cameraPos = { 1.0f,1.0f,-10.0f };
glm::vec3 lookAtCam = { 0.0f,0.0f,0.0f };
glm::vec3 vecUp = { 0.0f,1.0f,0.0f };
Camera camera = Camera(cameraPos, lookAtCam, vecUp);

#pragma endregion

using namespace std;

int main(void) {
	//Set Console Pos
	SetWindowPos(GetConsoleWindow(), 0, WIDTH - 7, -11, 0, 0, SWP_NOSIZE);

	//Hide Cursor
	ShowCursor(false);//Oculta-se o cursor

	/*Criação do Contexto OpenGL*/
	if (!glfwInit()) return -1;


	//Create window
	if (!(window.NewWindow(WIDTH, HEIGHT, (char*)"TBP3D", NULL, NULL)))
		return -1;

	//Set Window Pos
	window.SetWindowPos(0, 0);

	//MakeContextCurrent
	window.MakeContextCurrent();

	/*Os inputs e respetivos controladores são associados à janela ativa*/
	gameUserInputs.AssociateWindow(window.windowPtr, WIDTH, HEIGHT);
	controller.AssocieateUserInput(&gameUserInputs);

	glewInit();//Inicializa-se a GLEW

	loadProgramShader();//Carregam-se os diversos shaders

	LoadObjects();//Carregam-se os diversos objetos e respetivas informações

	while (!glfwWindowShouldClose(window.windowPtr)) {

		/*Inicia-se um ciclo*/
		clock_t begin = clock();

		Update();
		Draw();

		glfwSwapBuffers(window.windowPtr);
		glfwPollEvents();
		clock_t end = clock();

		/*Dada a contagem de tempo pela função clock(), o valor respetivo aos segundos passados é transferido para o Update*/
		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		Time::Update(elapsed_secs);

	}
}

void loadProgramShader()
{
	//Load cube shader
	ShaderInfo  cubeShaders[] = {
		{ GL_VERTEX_SHADER,   "shaders/cubeShader/cube.vert" },
		{ GL_FRAGMENT_SHADER, "shaders/cubeShader/cube.frag" },
		{ GL_NONE, NULL }
	};
	cubeProgramShader = LoadShaders(cubeShaders);
	if (!cubeProgramShader) exit(EXIT_FAILURE);



	//Load Iron Man shader
	ShaderInfo  ironManshaders[] = {
		{ GL_VERTEX_SHADER,   "shaders/ironMan/ironMan.vert" },
		{ GL_FRAGMENT_SHADER, "shaders/ironMan/ironMan.frag" },
		{ GL_NONE, NULL }
	};
	ironManProgramShader = LoadShaders(ironManshaders);
	if (!ironManProgramShader) exit(EXIT_FAILURE);


}

void LoadObjects() {
	glClearColor(0.11f, 0.11f, 0.11f, 0.0f);

	/* Informação sobre o objeto cubo */
	vector<glm::vec3> vertexPos = {
	{0.5f, -0.5f,  0.5f},
	{0.5f, -0.5f, -0.5f},
	{0.5f,  0.5f,  0.5f},

	{0.5f,  0.5f,  0.5f},
	{0.5f, -0.5f, -0.5f},
	{0.5f,  0.5f, -0.5f},

	{-0.5f, -0.5f, -0.5f},
	{-0.5f, -0.5f,  0.5f},
	{-0.5f,  0.5f, -0.5f},

	{-0.5f,  0.5f, -0.5f},
	{-0.5f, -0.5f,  0.5f},
	{-0.5f,  0.5f,  0.5f},

	{-0.5f,  0.5f,  0.5f},
	{ 0.5f,  0.5f,  0.5f},
	{-0.5f,  0.5f, -0.5f},

	{-0.5f,  0.5f, -0.5f},
	{ 0.5f,  0.5f,  0.5f},
	{ 0.5f,  0.5f, -0.5f},

	{-0.5f, -0.5f, -0.5f},
	{ 0.5f, -0.5f, -0.5f},
	{-0.5f, -0.5f,  0.5f},

	{-0.5f, -0.5f,  0.5f},
	{ 0.5f, -0.5f, -0.5f},
	{ 0.5f, -0.5f,  0.5f},

	{-0.5f, -0.5f, 0.5f},
	{ 0.5f, -0.5f, 0.5f},
	{-0.5f,  0.5f, 0.5f},

	{-0.5f,  0.5f, 0.5f},
	{ 0.5f, -0.5f, 0.5f},
	{ 0.5f,  0.5f, 0.5f},

	{ 0.5f, -0.5f, -0.5f},
	{-0.5f, -0.5f, -0.5f},
	{ 0.5f,  0.5f, -0.5f},

	{ 0.5f,  0.5f, -0.5f},
	{-0.5f, -0.5f, -0.5f},
	{-0.5f,  0.5f, -0.5f}
	};
	vector<glm::vec3> vertexColor;
	srand(31);//6,11,31
	for (int i = 0; i < 6; i++) {

		float a = ((float)rand()) / (float)RAND_MAX;
		float b = ((float)rand()) / (float)RAND_MAX;
		float c = ((float)rand()) / (float)RAND_MAX;

		for (int j = 0; j < 6; j++) {
			vertexColor.push_back(glm::vec3(a, b, c));
		}
	}
	//Indexed Cube Vertex

	vector<vec3> cube_vertices = {
		// front
		{-1.0, -1.0,  1.0},
		{ 1.0, -1.0,  1.0},
		{ 1.0,  1.0,  1.0},
		{-1.0,  1.0,  1.0},
		// back		 
		{-1.0, -1.0, -1.0},
		{ 1.0, -1.0, -1.0},
		{ 1.0,  1.0, -1.0},
		{-1.0,  1.0, -1.0},
	};

	vector<vec3> cube_colors = {
		// Cores de frente
		{1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{0.0, 0.0, 1.0},
		{1.0, 1.0, 1.0},
		// Cores do Fundo
		{1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{0.0, 0.0, 1.0},
		{1.0, 1.0, 1.0},
	};

	vector<GLuint> cube_elements = {
		// front
		0, 1, 2,
		2, 3, 0,
		// top
		3, 2, 6,
		6, 7, 3,
		// back
		7, 6, 5,
		5, 4, 7,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// left
		4, 0, 3,
		3, 7, 4,
		// right
		1, 5, 6,
		6, 2, 1,
	};

	/*Cria-se um objeto, ao qual são atribuídos um programa shader, um array de vértices e um array de cores*/
	cube2.CreateObjPosColor(cubeProgramShader, &cube_vertices, &cube_colors);
	cube2.meshRenderer.mesh.AssignElementArray(&cube_elements);
	cube2.transform.SetPos(vec3(3, 10, 0));
	cube2.transform.scale = vec3(0.5f, 0.5f, 0.5f);


	int index = 0;
	for (int h = 0; h < 10; h++)
		for (int i = 0; i < 10; i++)
		{
			cubes[index].CreateObjPosColor(cubeProgramShader, &vertexPos, &vertexColor);
			cubes[index].transform.SetPos(vec3((i * 1.5f) - 4, -1, (h * 1.5f) - 6.0f));
			index++;
		}

	/*Altera-se a posição do objeto*/
	cube.transform.SetPos(glm::vec3(3, 2, 0));

	/*Informação sobre o objeto Iron Man*/
	{
		ironMan.LoadObjFile(ironManProgramShader, "gameObjects/Iron_Man/", "Iron_Man.obj");
		ironMan2.LoadObjFile(ironManProgramShader, "gameObjects/Iron_Man/", "Iron_Man.obj");

		ironManTexture.TesxureToProgram(ironManProgramShader, "gameObjects/Iron_Man/Iron_Man_D.tga");
		ironMan2.transform.SetPos(vec3(6, 0, 0));
	}

	/*Adicionam-se as diversas fontes de luz ao programa shader do objeto*/
	lights.AddAmbientLight(ironManProgramShader, vec3(0.1f, 0.1f, 0.1f));
	lights.AddDirectionalLight(ironManProgramShader, vec3(1, 0, 0), vec3(0.2, 0.2, 0.2), vec3(0, 0.5, 1)*2.0f, vec3(0, 0.5, 1)*100.0f);
	lights.AddPointLight(ironManProgramShader, vec3(1.0, 2.0, 0.0), vec3(0.1, 0.1, 0.1), vec3(1.0, 0, 0.1)*2.0f, vec3(1.0, 0, 0.1)*10.0f, 1, 0.06f, 0.002f);
	lights.AddSpotLight(ironManProgramShader, vec3(0, 3, 2), vec3(0, 0, -2), vec3(0.1, 0.1, 0.1), vec3(1, 1, 1)*2.0f, vec3(1.0, 1.0, 1.0), 1, 0.006f, 0.002f, (SMALL_PI / 12));

	glViewport(0, 0, WIDTH, HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); // Por defeito está desativado
	glCullFace(GL_BACK); // GL_FRONT, [GL_BACK], GL_FRONT_AND_BACK
}

void Update()
{


	float deltaTime = Time::deltaTime;

	/*Aplica-se o movimento base na camera*/
	controller.BasicMoviment(&camera.transform, 0.15f*deltaTime, 5 * deltaTime);

	/*Aplicam-se as rotações aos objetos*/
	//cube.transform.Rotate(vec3(-1, -1, 0)*2.0f*deltaTime);
	ironMan.transform.Rotate(vec3(1, 0, 0)*deltaTime);
	ironMan2.transform.Rotate(vec3(-1, 0, 0)*deltaTime);


	/*Ativa/Desativa a deformação do modelo*/
	if (gameUserInputs.onceKey5)
	{
		cube2.transform.Rotate(vec3(1, 1, 0)*2.0f*deltaTime);
		cube2.transform.scale = vec3(cos(Time::time*20)+1, cos(Time::time * 20)+1, cos(Time::time * 20)+1);

		for (int i = 0; i < 100; i++)
		{
			cubes[i].transform.Rotate(vec3(1 + (i*0.0005f), 0, 0)*(deltaTime));
			cubes[i].transform.scale = vec3(1,1,1)*(cos(Time::time*2) + 3)*0.3f;
		}

		offsetToggle = 1;
	}
	else
		offsetToggle = 0;





	//offsetToggle é passado ao programa shader como uniform
	glProgramUniform1i(ironManProgramShader, glGetProgramResourceLocation(ironManProgramShader, GL_UNIFORM, "offsetToggle"), offsetToggle);

	if (gameUserInputs.key6) renderMode = GL_TRIANGLES;
	if (gameUserInputs.key7) renderMode = GL_LINE_STRIP;
	if (gameUserInputs.key8) renderMode = GL_POINTS;
	if (gameUserInputs.key9) renderMode = GL_TRIANGLE_FAN;


	/*Se pressionadas as teclas de 1 a 4, as respetivas luzes são ativadas/desativadas*/
	(gameUserInputs.onceKey1) ? lights.ToggleAmbientLight(ironManProgramShader, true) : lights.ToggleAmbientLight(ironManProgramShader, false);
	(gameUserInputs.onceKey2) ? lights.ToggleDirectionalLight(ironManProgramShader, true) : lights.ToggleDirectionalLight(ironManProgramShader, false);
	(gameUserInputs.onceKey3) ? lights.TogglePointLight(ironManProgramShader, 0, true) : lights.TogglePointLight(ironManProgramShader, 0, false);
	(gameUserInputs.onceKey4) ? lights.ToggleSpotLight(ironManProgramShader, 0, true) : lights.ToggleSpotLight(ironManProgramShader, 0, false);


	controller.Exit(window.windowPtr);
}

void Draw() {

	//Antes da chamada do desenho de primitivas, os buffers deverão ser limpos
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*Passam-se os valores do tempo aos programas shader*/
	Time::TimeToProgram(ironManProgramShader);
	Time::TimeToProgram(cubeProgramShader);

	/*Renderizam-se os diversos objetos*/
	ironMan.meshRenderer.Draw(renderMode, &camera);
	ironMan2.meshRenderer.Draw(renderMode, &camera);
	//cube.meshRenderer.Draw(renderMode,&camera);
	cube2.meshRenderer.Draw(renderMode, &camera);

	for (int i = 0; i < 100; i++)
		cubes[i].meshRenderer.Draw(renderMode, &camera);

}



