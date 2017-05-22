#version 330 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D screenTexture;

void main()
{ 
	//compute the normal without interpolation.
	//vec3 normal = normalize(cross(dFdx(position), dFdy(position)));

    color = texture(screenTexture, TexCoords);
   // color = vec4(1);
}