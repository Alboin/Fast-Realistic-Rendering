#pragma region Includes.

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL/src/SOIL.h>

#include <iostream>
#include <vector>
#include <string>
#include <math.h>

#include "shaderLoader.h"
#include "plyloader.h"
#include "PLYDrawer.h"
#include "CubeMap.h"
#include "SolidSphere.cpp"
#include "Framebuffer.h"

#pragma endregion

using namespace glm;
using namespace std;

#pragma region Global variables.
// Global variables for rendering
glm::mat4 view;
glm::mat4 projection;
GLuint shaderProgramID;
GLuint skyboxShaderID;
GLuint quadShaderID;
PLYDrawer *mesh;

// Global variables for controls
bool leftMousePressed = false;
double mouseX, mouseY;
int roughness = 0;
int maxRoughness = 6;
float fresnelR = 0.4f, fresnelG = 0.4f, fresnelB = 0.4f;
int reflection = 0;
bool drawSphere = false;
int wireframe = 0;
bool exitProgram = false;

// Global variables for the fps-counter
double t0 = 0.0;
int frames = 0;
char titlestring[200];
#pragma endregion

// Function declarations
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void updateMeshUniforms();
void updateSkyboxUniforms();


int main()
{

	#pragma region initiation
	int windowWidth = 1200;
	int windowHeight = 800;

	//Starting position of camera
	view = lookAt(vec3(0.0f, 0.0f, 2.0f), vec3(0, 0, 0), vec3(0, 1, 0));
	//Projection matrix
	projection = glm::perspective(45.0f, (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

	//Initiate glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	//Try to create a window
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "FRR", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//Initiate glew
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	//Tell OpenGL the size of the viewport
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glViewport(0, 0, width, height);


	// Create and compile the GLSL program from the shaders
	shaderProgramID = LoadShaders("vertexshader.glsl", "fragmentshader.glsl");
	skyboxShaderID = LoadShaders("cubemapVert.glsl", "cubemapFrag.glsl");
	quadShaderID = LoadShaders("quadVert.glsl", "quadFrag.glsl");

	//Register external intpu in GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	#pragma endregion

	//Load ply-model
	PLYModel plymodel("models/bunny.ply", false, false);

	GLuint VBO, VAO, EBO;
	mesh = new PLYDrawer(plymodel, VBO, VAO, EBO);

	GLuint VBO_sphere, VAO_sphere, EBO_sphere;
	SolidSphere sphere(0.5f, 100, 100, VBO_sphere, VAO_sphere, EBO_sphere);

	//Create cubemap
	GLuint VBO_map, VAO_map, EBO_map;
	CubeMap skybox(VBO_map, VAO_map, EBO_map, maxRoughness);

	//Create framebuffer
	Framebuffer framebuffer(width, height);

	//Rendering commands here
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPointSize(5);
	glEnable(GL_TEXTURE_CUBE_MAP);
	//glDepthMask(GL_TRUE);


	while (!glfwWindowShouldClose(window) && !exitProgram)
	{
		//Checks if any events are triggered (like keyboard or mouse events)
		glfwPollEvents();
		//Update mouse position
		glfwGetCursorPos(window, &mouseX, &mouseY);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw to framebuffer
		framebuffer.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (wireframe == 0)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else if (wireframe == 1)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);


		//Use the skybox-shader.
		glUseProgram(skyboxShaderID);
		//RENDERING SKYBOX HERE
		updateSkyboxUniforms();
		skybox.drawCubeMap(skyboxShaderID);


		//Use the mesh-shader.
		glUseProgram(shaderProgramID);
		////RENDERING MESH HERE
		updateMeshUniforms();


		if (drawSphere)
			sphere.drawSphere(shaderProgramID, skybox.textures[maxRoughness], skybox.textures[roughness + 1]);
		else
			mesh->drawPlyModel(shaderProgramID, skybox.textures[maxRoughness], skybox.textures[roughness + 1]);


		// Unbind framebuffer.
		framebuffer.unbind();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//Draw to quad.
		framebuffer.drawToQuad(quadShaderID);

		//Swap the buffers
		glfwSwapBuffers(window);

		#pragma region fps-counter
		//Show fps in window title
		double t = glfwGetTime();
		// If one second has passed, or if this is the very first frame
		if ((t - t0) > 1.0 || frames == 0)
		{
			double fps = (double)frames / (t - t0);
			sprintf(titlestring, "FRR (%.1f fps)", fps);
			glfwSetWindowTitle(window, titlestring);
			t0 = t;
			frames = 0;
		}
		frames++;
		#pragma endregion

	}

	glfwTerminate();
	plymodel.FreeMemory();
	delete mesh;

	return 0;
}

void updateMeshUniforms()
{
	glm::mat4 model = mat4(1.0f);
	if (!drawSphere)
		//"Normalize" the model scale.
		model = glm::scale(mat4(1.0f), vec3(1/mesh->height, 1/mesh->height, 1/mesh->height));
	//Center the model at origo.
	model = glm::translate(model, vec3(0, -mesh->height/2 - mesh->minPos.y, 0));

	vec3 cameraPos = (inverse(view))[3];

	//Send all matrixes needed to mesh shaders.
	GLuint viewtransLoc1 = glGetUniformLocation(shaderProgramID, "viewMesh");
	glUniformMatrix4fv(viewtransLoc1, 1, GL_FALSE, glm::value_ptr(view));
	GLuint modeltransLoc1 = glGetUniformLocation(shaderProgramID, "modelMesh");
	glUniformMatrix4fv(modeltransLoc1, 1, GL_FALSE, glm::value_ptr(model));
	GLuint projectiontransLoc1 = glGetUniformLocation(shaderProgramID, "projectionMesh");
	glUniformMatrix4fv(projectiontransLoc1, 1, GL_FALSE, glm::value_ptr(projection));
	GLint fresnelRLoc = glGetUniformLocation(shaderProgramID, "fresnelR");
	glUniform1f(fresnelRLoc, fresnelR);
	GLint fresnelGLoc = glGetUniformLocation(shaderProgramID, "fresnelG");
	glUniform1f(fresnelGLoc, fresnelG);
	GLint fresnelBLoc = glGetUniformLocation(shaderProgramID, "fresnelB");
	glUniform1f(fresnelBLoc, fresnelB);
	GLint reflectionLoc = glGetUniformLocation(shaderProgramID, "reflection");
	glUniform1i(reflectionLoc, reflection);
	GLint cameraPosLoc1 = glGetUniformLocation(shaderProgramID, "cameraPos");
	glUniform3f(cameraPosLoc1, cameraPos.x, cameraPos.y, cameraPos.z);
}

void updateSkyboxUniforms()
{
	glm::mat4 cubeModel = glm::scale(mat4(1.0f), vec3(10.0f));
	cubeModel = rotate(cubeModel, 3.14f / 2.0f, vec3(0, 1, 0));
	mat4 cubeView = mat4(mat3(view));

	//Send all matrixes needed to cube shaders.
	GLuint viewtransLoc2 = glGetUniformLocation(skyboxShaderID, "viewCube");
	glUniformMatrix4fv(viewtransLoc2, 1, GL_FALSE, glm::value_ptr(cubeView));
	GLuint modeltransLoc2 = glGetUniformLocation(skyboxShaderID, "modelCube");
	glUniformMatrix4fv(modeltransLoc2, 1, GL_FALSE, glm::value_ptr(cubeModel));
	GLuint projectiontransLoc2 = glGetUniformLocation(skyboxShaderID, "projectionCube");
	glUniformMatrix4fv(projectiontransLoc2, 1, GL_FALSE, glm::value_ptr(projection));
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//By pressing D/S the user can make the model more diffuse/specular
	if (key == GLFW_KEY_D && action == GLFW_PRESS && roughness < maxRoughness)
		roughness++;
	else if (key == GLFW_KEY_S && action == GLFW_PRESS && roughness > 0)
		roughness--;

	//By pressing W/E the user can decrease/increase the fresnel constant
	if (key == GLFW_KEY_W)
	{
		fresnelR -= 0.1f;
		fresnelG -= 0.1f;
		fresnelB -= 0.1f;
	}
	else if (key == GLFW_KEY_E)
	{
		fresnelR += 0.1f;
		fresnelG += 0.1f;
		fresnelB += 0.1f;
	}
	//The following keys decrease/increase the separate fresnel constants for R, G and B.
	if (key == GLFW_KEY_F)
		fresnelR -= 0.1f;
	else if (key == GLFW_KEY_R)
		fresnelR += 0.1f;
	if (key == GLFW_KEY_G)
		fresnelG -= 0.1f;
	else if (key == GLFW_KEY_T)
		fresnelG += 0.1f;
	if (key == GLFW_KEY_H)
		fresnelB -= 0.1f;
	else if (key == GLFW_KEY_Y)
		fresnelB += 0.1f;

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		if (reflection == 1)
			reflection = 0;
		else
			reflection = 1;
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
		if (drawSphere)
			drawSphere = false;
		else
			drawSphere = true;

	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
	{
		wireframe++;
		if (wireframe > 2)
			wireframe = 0;
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		exitProgram = true;
	}
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (leftMousePressed)
	{
		//Rotate around Y-axis
		view = rotate(view, (float)(xpos - mouseX) / 100, vec3(0.0f, 1.0f, 0.0f));

		//Rotate around up/down
		mat3 rotMat(view);
		vec3 transl(view[3]);
		vec3 camera_pos = -transl * rotMat;

		camera_pos = vec3(camera_pos.x, 0.0f, camera_pos.z);
		vec3 temp = normalize(cross(vec3(0.0f, 1.0f, 0.0f), camera_pos));
		view = rotate(view, (float)(ypos - mouseY) / 150, temp);
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//Zoom in and out through scrolling
	view = scale(view, vec3(1.0 + 0.1*yoffset));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		leftMousePressed = true;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		leftMousePressed = false;
}