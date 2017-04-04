#include "CubeMap.h"
#include <string>
#include <iostream>

CubeMap::CubeMap(GLuint &VBO, GLuint &VAO, GLuint &EBO, const int maxRoughness)
{
	vector<const GLchar*> faces;

	//Add faces for the skybox
	faces.push_back("textures/sky/right.jpg");
	faces.push_back("textures/sky/left.jpg");
	faces.push_back("textures/sky/top.jpg");
	faces.push_back("textures/sky/bottom.jpg");
	faces.push_back("textures/sky/back.jpg");
	faces.push_back("textures/sky/front.jpg");
	//Save as first texture in "textures".
	GLuint cubemapTexture = loadCubemap(faces);
	textures.push_back(cubemapTexture);
	faces.clear();

	//Add faces for cubemap reflection.
	for (int j = 0; j <= maxRoughness; j++)
	{
		for (int i = 0; i < 6; i++)
		{
			string loc = "textures/sky/";
			loc.append(to_string(j));
			loc.append("_c0");
			loc.append(to_string(i));
			loc.append(".jpg");
			char *cstr = new char[loc.length() + 1];
			strcpy(cstr, loc.c_str());
			faces.push_back(cstr);
		}
		GLuint cubemapTexture = loadCubemap(faces);
		textures.push_back(cubemapTexture);

		faces.clear();
	}

	createVertices();
	createVBO(VBO, VAO, EBO);
}

void CubeMap::createVBO(GLuint &VBO, GLuint &VAO, GLuint &EBO)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ivec3)*indexes.size(), &indexes[0], GL_STATIC_DRAW);

	//Vertex position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Save the adresses.
	copyVBO = VBO;
	copyVAO = VAO;
	copyEBO = EBO;
}

void CubeMap::drawCubeMap(GLuint shaderProgramID)
{
	glDepthMask(GL_FALSE);
	glUseProgram(shaderProgramID);
	glBindBuffer(GL_ARRAY_BUFFER, copyVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

	GLuint tex0 = glGetUniformLocation(shaderProgramID, "cubemap");
	glUniform1i(tex0, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[0]);
	glBindVertexArray(copyVAO);
	glDrawElements(GL_TRIANGLES, sizeof(ivec3)*indexes.size(), GL_UNSIGNED_INT, 0);
	glDepthMask(GL_TRUE);
	glBindVertexArray(0);
}


GLuint CubeMap::loadCubemap(vector<const GLchar*> faces)
{
	//Create a texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	int width, height;
	unsigned char* image;

	//Load the different textures through SOIL
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
		);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}

void CubeMap::createVertices()
{
	GLfloat positions[] = {
		1.0, 1.0, 1.0, //0
		1.0, 1.0, -1.0, //1
		1.0, -1.0, 1.0, //2
		-1.0, 1.0, 1.0, //3
		1.0, -1.0, -1.0, //4
		-1.0, 1.0, -1.0, //5
		-1.0, -1.0, 1.0, //6
		-1.0, -1.0, -1.0 //7
	};

	GLuint indx[] = {
		5, 7, 4, //back1
		4, 1, 5, //back2
		6, 7, 5, //left1
		5, 3, 6, //left2
		4, 2, 0, //right1
		0, 1, 4, //right2
		6, 3, 0, //front1
		0, 2, 6, //front2
		5, 1, 0, //top1
		0, 3, 5, //top2
		7, 6, 4, //bottom1
		4, 6, 2 //bottom2
	};

	for (int i = 0; i < 8 * 3; i += 3)
		vertices.push_back(vec3(positions[i], positions[i + 1], positions[i + 2]));
		
	for (int i = 0; i < 12 * 3; i += 3)
	{
		indexes.push_back(vec3(indx[i], indx[i + 1], indx[i + 2]));
		//normals.push_back(vec3(nrmls[i], nrmls[i + 1], nrmls[i + 2]));
	}

}