#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL/src/SOIL.h>

using namespace std;
using namespace glm;

class CubeMap
{
public:
	CubeMap(GLuint &VBO, GLuint &VAO, GLuint &EBO, const int maxRoughness);
	void drawCubeMap(GLuint shaderProgramID);
	
	vector<GLuint> textures;

private:
	void createVertices();
	void createVBO(GLuint &VBO, GLuint &VAO, GLuint &EBO);
	GLuint loadCubemap(vector<const GLchar*> faces);

	vector<vec3> vertices;
	//vector<vec3> normals;
	vector<ivec3> indexes;

	GLuint copyVBO, copyVAO, copyEBO;

};