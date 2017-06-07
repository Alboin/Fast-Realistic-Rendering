#version 330 core

in vec3 vertexNormal;
in vec3 fragPos;
in vec4 gl_FragCoord;

out vec4 color;


uniform mat4 modelMesh;
uniform mat4 viewMesh;

void main()
{
	float near = 0.1f;
	float far = 5.0f;

	//compute a normal without interpolation (as in flat shading).
	vec3 normal = normalize(cross(dFdx(fragPos), dFdy(fragPos)));

	float z = (2 * near) / (far + near - gl_FragCoord.z * (far - near));
	
	// Save the normals and the linearized depth to vec4 color.
	color = vec4(normal, z);
}