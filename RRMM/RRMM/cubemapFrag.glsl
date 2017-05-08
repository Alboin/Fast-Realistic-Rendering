#version 330 core

in vec3 texCoords;

uniform samplerCube cubemap;

out vec4 color;

void main()
{
	color = texture(cubemap, texCoords);
}