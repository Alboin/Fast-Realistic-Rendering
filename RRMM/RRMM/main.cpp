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

using namespace glm;
using namespace std;

// Global variables for rendering and controls
mat4 view;
bool leftMousePressed = false;
double mouseX, mouseY;
int roughness = 0;
int maxRoughness = 6;
float fresnelR = 0.4f, fresnelG = 0.4f, fresnelB = 0.4f;
int reflection = 0;
bool drawSphere = false;

// Global variables for the fps-counter
double t0 = 0.0;
int frames = 0;
char titlestring[200];

// Function declarations
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


int main()
{

	#pragma region initiation
	int windowWidth = 1200;
	int windowHeight = 800;

	//Starting position of camera
	view = lookAt(vec3(0.0f, 0.0f, 2.0f), vec3(0, 0, 0), vec3(0, 1, 0));
	//Projection matrix
	glm::mat4 projection;
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
	GLuint shaderProgramID = LoadShaders("vertexshader.glsl", "fragmentshader.glsl");
	GLuint skyboxShaderID = LoadShaders("cubemapVert.glsl", "cubemapFrag.glsl");
	GLuint quadShaderID = LoadShaders("quadVert.glsl", "quadFrag.glsl");

	//Register external intpu in GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	#pragma endregion

	//Load ply-model
	PLYModel plymodel("models/bunny.ply", false, false);

	GLuint VBO, VAO, EBO;
	PLYDrawer mesh(plymodel, VBO, VAO, EBO);

	GLuint VBO_sphere, VAO_sphere, EBO_sphere;
	SolidSphere sphere(0.5f, 100, 100, VBO_sphere, VAO_sphere, EBO_sphere);

	//Create cubemap
	GLuint VBO_map, VAO_map, EBO_map;
	CubeMap skybox(VBO_map, VAO_map, EBO_map, maxRoughness);

	//Create framebuffer
	GLuint FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	//Create a texture for the framebuffer
	GLuint framebufferTexture;
	glGenTextures(1, &framebufferTexture);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Attach texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

	//Check that the framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR: Framebuffer is not complete!" << endl;

	#pragma region Create quad to render final image to.
	//Create quad to render final image to.
	GLfloat quadVertices[] = {
		// Positions   // TexCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	GLuint VAO_quad, VBO_quad;
	glGenVertexArrays(1, &VAO_quad);
	glGenBuffers(1, &VBO_quad);
	glBindVertexArray(VAO_quad);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_quad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	//Vertex position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	//Texture coordinate attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2*sizeof(GLfloat)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	#pragma endregion

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	while (!glfwWindowShouldClose(window))
	{
		//Checks if any events are triggered (like keyboard or mouse events)
		glfwPollEvents();
		//Update mouse position
		glfwGetCursorPos(window, &mouseX, &mouseY);

		//Draw to framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		//Rendering commands here
		glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//Use the mesh-shader.
		glUseProgram(shaderProgramID);

		#pragma region MVP-matrixes for mesh
		glm::mat4 model = mat4(1.0f);
		if (!drawSphere)
			//"Normalize" the model scale.
			model = glm::scale(mat4(1.0f), vec3(1/mesh.height, 1/mesh.height, 1/mesh.height));
		//Center the model at origo.
		model = glm::translate(model, vec3(0, -mesh.height/2 - mesh.minPos.y, 0));

		vec3 cameraPos = (inverse(view))[3];
		#pragma endregion

		#pragma region Uniforms for mesh
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
		#pragma endregion

		//RENDERING MESH HERE
		if (drawSphere)
			sphere.drawSphere(shaderProgramID, skybox.textures[maxRoughness], skybox.textures[roughness + 1]);
		else
			mesh.drawPlyModel(shaderProgramID, skybox.textures[maxRoughness], skybox.textures[roughness + 1]);


		//Use the skybox-shader.
		glUseProgram(skyboxShaderID);

		#pragma region model-matrix for cube
		glm::mat4 cubeModel = glm::scale(mat4(1.0f), vec3(10.0f));
		cubeModel = rotate(cubeModel, 3.14f / 2.0f, vec3(0, 1, 0));
		mat4 cubeView = mat4(mat3(view));
		#pragma endregion

		#pragma region Uniforms for skybox
		//Send all matrixes needed to cube shaders.
		GLuint viewtransLoc2 = glGetUniformLocation(skyboxShaderID, "viewCube");
		glUniformMatrix4fv(viewtransLoc2, 1, GL_FALSE, glm::value_ptr(cubeView));
		GLuint modeltransLoc2 = glGetUniformLocation(skyboxShaderID, "modelCube");
		glUniformMatrix4fv(modeltransLoc2, 1, GL_FALSE, glm::value_ptr(cubeModel));
		GLuint projectiontransLoc2 = glGetUniformLocation(skyboxShaderID, "projectionCube");
		glUniformMatrix4fv(projectiontransLoc2, 1, GL_FALSE, glm::value_ptr(projection));
		#pragma endregion

		//RENDERING SKYBOX HERE
		skybox.drawCubeMap(skyboxShaderID);



		//Draw to quad.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glUseProgram(quadShaderID);
		glBindVertexArray(VAO_quad);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, framebufferTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);



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

	return 0;
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