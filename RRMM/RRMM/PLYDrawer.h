#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "plyloader.h"

using namespace std;
using namespace glm;

class PLYDrawer
{
public:
	PLYDrawer(const PLYModel &ply, GLuint &VBO, GLuint &VAO, GLuint &EBO);
	void drawPlyModel(GLuint shaderProgramID, GLuint diffuseTexID, GLuint specularTexID);
	void computeNormals();
	float width, height, depth;
	vec3 minPos;
	vec3 maxPos;
private:
	vector<vec3> vboArray;
	vector<ivec3> indices;
	PLYModel model;

	GLuint copyVBO, copyVAO, copyEBO;
};