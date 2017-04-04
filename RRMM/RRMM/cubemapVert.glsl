#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 modelCube;
uniform mat4 projectionCube;
uniform mat4 viewCube;

out vec3 texCoords;

void main()
{
	gl_Position = projectionCube * viewCube * modelCube * vec4(position, 1.0f);

	texCoords = position;
}