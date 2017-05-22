#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

using namespace glm;
using namespace std;

class Framebuffer
{
private:

	GLuint FBO;
	GLuint VAO_quad, VBO_quad;
	GLuint framebufferTexture;
	GLuint depthrenderbuffer;
	GLuint rbo;

	int width, height;

	void createDepthBuffer();


	//Create quad to render final image to.
	GLfloat quadVertices[24] = {
		// Positions   // TexCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

public:
	Framebuffer(int windowWidth, int windowHeight);
	~Framebuffer();

	void bind();
	void unbind();
	void drawToQuad(GLuint quadShaderID);




};

