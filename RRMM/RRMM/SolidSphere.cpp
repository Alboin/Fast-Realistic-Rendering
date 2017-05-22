#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <vector>
#include <iostream>
#include <cmath>

using namespace std;
using namespace glm;

class SolidSphere
{
protected:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;
	std::vector<GLushort> indices;
	std::vector<glm::vec3> vboArray;
	std::vector<glm::ivec3> indicesNew;
	float r;

	GLuint copyVBO, copyVAO, copyEBO;


public:
	SolidSphere(float radius, unsigned int rings, unsigned int sectors, GLuint &VBO, GLuint &VAO, GLuint &EBO)
		:r(radius)
	{
		float const R = 1. / (float)(rings - 1);
		float const S = 1. / (float)(sectors - 1);
		int r, s;

		const float M_PI = 3.14159265359;

		vertices.resize(rings * sectors * 3);
		normals.resize(rings * sectors * 3);
		texcoords.resize(rings * sectors * 2);
		std::vector<glm::vec3>::iterator v = vertices.begin();
		std::vector<glm::vec3>::iterator n = normals.begin();
		std::vector<glm::vec2>::iterator t = texcoords.begin();
		for (r = 0; r < rings; r++) for (s = 0; s < sectors; s++) {
			float const y = sin((-1)*(M_PI / 2) + M_PI * r * R);
			float const x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
			float const z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);

			*t++ = glm::vec2(s*S, r*R);

			*v++ = glm::vec3(x * radius, y * radius, z * radius);

			*n++ = glm::vec3(x, y, z);
		}

		indices.resize(rings * sectors * 6);
		std::vector<GLushort>::iterator i = indices.begin();
		for (r = 0; r < rings - 1; r++) for (s = 0; s < sectors - 1; s++) {
			*i++ = r * sectors + s;
			*i++ = (r + 1) * sectors + s;
			*i++ = (r + 1) * sectors + (s + 1);

			*i++ = r * sectors + s;
			*i++ = (r + 1) * sectors + (s + 1);
			*i++ = r * sectors + (s + 1);

		}
		//std::cout << "VERTICES: " << vertices.size() << "NORMALS:" << normals.size();
		for (int i = 0; i < vertices.size(); i++)
		{
			vboArray.push_back(vertices[i]);
			//std::cout << vertices[i].x << "," << vertices[i].y << "," << vertices[i].z;
			vboArray.push_back(normals[i]);
			vboArray.push_back(vec3(1, 0.2, 0)); //vertex color
		}
		for (int i = 0; i < indices.size(); i += 3)
		{
			indicesNew.push_back(ivec3(indices[i]*3, indices[i + 1]*3, indices[i + 2]*3));
		}
		

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*vboArray.size(), &vboArray[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ivec3)*indicesNew.size(), &indicesNew[0], GL_STATIC_DRAW);

		//Vertex position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(vec3), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		//Vertex normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(vec3), (GLvoid*)(sizeof(vec3)));
		glEnableVertexAttribArray(1);
		//Vertex color attribute
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(vec3), (GLvoid*)(2 * sizeof(vec3)));
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		
		// Save the adresses.
		copyVBO = VBO;
		copyVAO = VAO;
		copyEBO = EBO;
	}

	void drawSphere(const GLuint& shaderProgramID, const GLuint&  diffuseTexID, const GLuint&  specularTexID)
	{
		glBindBuffer(GL_ARRAY_BUFFER, copyVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vboArray.size(), &vboArray[0], GL_STATIC_DRAW);
		glBindVertexArray(copyVAO);

		GLuint tex0 = glGetUniformLocation(shaderProgramID, "diffuseCube");
		glUniform1i(tex0, 0);
		GLuint tex1 = glGetUniformLocation(shaderProgramID, "specularCube");
		glUniform1i(tex1, 1);

		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, diffuseTexID);
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, specularTexID);

		glDrawElements(GL_TRIANGLES, sizeof(GLushort)*indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	}

	/*void draw(GLfloat x, GLfloat y, GLfloat z)
	{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(x, y, z);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
	glNormalPointer(GL_FLOAT, 0, &normals[0]);
	glTexCoordPointer(2, GL_FLOAT, 0, &texcoords[0]);
	glDrawElements(GL_QUADS, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);
	glPopMatrix();
	}*/
	std::vector<glm::vec3> getVertices()
	{
		return vertices;
	}
	std::vector<glm::vec3> getNormals()
	{
		return normals;
	}
	std::vector<glm::vec2> getTextcoords()
	{
		return texcoords;
	}
	std::vector<GLushort> getIndices()
	{
		return indices;
	}
	float getRadius()
	{
		return r;
	}
};