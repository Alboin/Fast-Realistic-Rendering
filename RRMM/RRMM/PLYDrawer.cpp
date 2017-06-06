#include "PLYDrawer.h"
#include <iostream>

PLYDrawer::PLYDrawer(const PLYModel &ply, GLuint &VBO, GLuint &VAO, GLuint &EBO)
	:model(ply)
{
	//Variables for the mesh's biggest/smallest coordinates in all directions.
	//Theses are used to normalize the scale of the model and center it on the screen.
	maxPos = vec3(-1000, -1000, -1000);
	minPos = vec3(1000, 1000, 1000);


	// Fill the vboArray with values.
	for (uint i = 0; i < model.vertexCount * 3; i++)
	{
		// Vertex positions
		if (i % 3 == 0)
		{
			vboArray.push_back(model.positions[i / 3]);
		}
		// Vertex normal, filled with temporary value.
		else if (i % 3 == 1)
		{
			//if (model.ifNormal)
				//vboArray.push_back(model.normals[i / 3]);
			//else
				vboArray.push_back(vec3(0, 0, 0));

		}
		// Vertex color
		else
		{
			vboArray.push_back(vec3(1, 0.2, 0));
		}

		//Save smallest/biggest values.
		if (model.positions[i / 3].x < minPos.x)
			minPos.x = model.positions[i / 3].x;
		if (model.positions[i / 3].y < minPos.y && abs(model.positions[i / 3].y) > 0.0001)
			minPos.y = model.positions[i / 3].y;
		if (model.positions[i / 3].z < minPos.z)
			minPos.z = model.positions[i / 3].z;

		if (model.positions[i / 3].x > maxPos.x)
			maxPos.x = model.positions[i / 3].x;
		if (model.positions[i / 3].y > maxPos.y)
			maxPos.y = model.positions[i / 3].y;
		if (model.positions[i / 3].z > maxPos.z)
			maxPos.z = model.positions[i / 3].z;
	}

	width = abs(maxPos.x - minPos.x);
	height = abs(maxPos.y - minPos.y);
	depth = abs(maxPos.z - minPos.z);

	for (int i = 0; i < model.faceCount; i++)
		indices.push_back(model.faces[i]);



	// Create floor.
	int currentLength = ((vboArray.size()) / 3) - 1;
	vboArray.push_back(vec3(maxPos.x * 3, minPos.y, maxPos.z * 3));
	vboArray.push_back(vec3(0,1,0));
	vboArray.push_back(vec3(1,1,1));

	vboArray.push_back(vec3(-maxPos.x * 3, minPos.y, maxPos.z * 3));
	vboArray.push_back(vec3(0,1,0));
	vboArray.push_back(vec3(1,1,1));

	vboArray.push_back(vec3(-maxPos.x * 3, minPos.y, -maxPos.z * 3));
	vboArray.push_back(vec3(0,1,0));
	vboArray.push_back(vec3(1,1,1));

	vboArray.push_back(vec3(maxPos.x * 3, minPos.y, -maxPos.z * 3));
	vboArray.push_back(vec3(0,1,0));
	vboArray.push_back(vec3(1,1,1));

	
	indices.push_back(ivec3(currentLength + 1, currentLength + 4, currentLength + 2));
	indices.push_back(ivec3(currentLength + 4, currentLength + 3, currentLength + 2));


	computeNormals();

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*vboArray.size(), &vboArray[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ivec3)*indices.size(), &indices[0], GL_STATIC_DRAW);

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

void PLYDrawer::drawPlyModel(GLuint shaderProgramID, GLuint diffuseTexID, GLuint specularTexID)
{
	glBindBuffer(GL_ARRAY_BUFFER, copyVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*vboArray.size(), &vboArray[0], GL_STATIC_DRAW);

	//GLuint tex0 = glGetUniformLocation(shaderProgramID, "diffuseCube");
	//glUniform1i(tex0, 0);
	//GLuint tex1 = glGetUniformLocation(shaderProgramID, "specularCube");
	//glUniform1i(tex1, 1);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, diffuseTexID);
	//glEnable(GL_TEXTURE_CUBE_MAP);
	//glActiveTexture(GL_TEXTURE1);
	//glEnable(GL_TEXTURE_CUBE_MAP);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, specularTexID);

	glBindVertexArray(copyVAO);

	glDrawElements(GL_TRIANGLES, sizeof(ivec3)*indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

}

void PLYDrawer::computeNormals()
{
	// If the model already has normals.
	if (model.ifNormal)
		return;

	// Calculate several normals for every vertex and add together.
	for (int i = 0; i < indices.size(); i++)
	{
		vec3 vertexA = vboArray[indices[i].x * 3];
		vec3 vertexB = vboArray[indices[i].y * 3];
		vec3 vertexC = vboArray[indices[i].z * 3];

		vec3 vecA = vertexC - vertexA;
		vec3 vecB = vertexC - vertexB;

		vec3 n = normalize(cross(vecA, vecB));

		vboArray[indices[i].x * 3 + 1] += n;
		vboArray[indices[i].y * 3 + 1] += n;
		vboArray[indices[i].z * 3 + 1] += n;
	}
	// Normalize all normals.
	for (int i = 1; i < vboArray.size(); i+=3)
	{
		vboArray[i] = normalize(vboArray[i]);
	}

}