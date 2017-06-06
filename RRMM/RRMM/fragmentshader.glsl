#version 330 core

in vec3 vertexNormal;
in vec3 fragPos;
in vec4 gl_FragCoord;

out vec4 color;

uniform vec3 cameraPos;
uniform float fresnelR;
uniform float fresnelG;
uniform float fresnelB;
uniform int reflection;

uniform mat4 modelMesh;
uniform mat4 viewMesh;

uniform samplerCube diffuseCube;
uniform samplerCube specularCube;

void main()
{
	float near = 0.1f;
	float far = 5.0f;

	//float z =  gl_FragCoord.z * 2.0f - 1.0f;
	//float linearDepth = (2.0 * near * far) / (far + near - z * (far - near));

	//compute a normal without interpolation (as in flat shading).
	vec3 normal = normalize(cross(dFdx(fragPos), dFdy(fragPos)));

	//vec3 a = normalize(cameraPos) * dot(normal, normalize(cameraPos));
	//vec3 b = normal - a;
	//normal = normalize(vec3(0,0,1) + b);

	vec4 temp = viewMesh * modelMesh * vec4(fragPos, 1.0f);
	//vec4 temp = viewMesh * modelMesh * gl_FragCoord;

	//float z = (2 * near) / (far + near - texture2D( texture0, texCoord ).x * (far - near));
	float z = (2 * near) / (far + near - gl_FragCoord.z * (far - near));

	//TODO: Remove "*100.0f", its only temporary!
	//float linearDepth = (temp.z - near)/(far - near) * 50.0f;
	float linearDepth = (temp.z - near)/(far - near);
	color = vec4(normal, z);

	//// DEBUG: Diffuse lighting
	//float diffuse = dot(normal, normalize(cameraPos));
	//color = vec4( vec3(1.0f * diffuse), 1.0f);

	//color = vec4(vec3(linearDepth), 1.0f);
	//color = vec4(vec3(vertexNormal.x, vertexNormal.y, vertexNormal.z), 1.0f);
	//color = vec4(vec3(vertexNormal.x, vertexNormal.y, vertexNormal.z), 1.0f) + vec4(vec3(linearDepth), 1.0f);	

	// Using the normal without interpolation.
	//color = vec4(vec3(normal.x, normal.y, normal.z), 1.0f) + vec4(vec3(linearDepth), 1.0f);
	return;	

	if(reflection != 1)
	{
		//Initiate variables
		vec3 v = normalize(fragPos - cameraPos);
		vec3 r = reflect(v, normalize(vertexNormal));
		vec3 n = vertexNormal;
		vec3 F0 = vec3(fresnelR, fresnelG, fresnelB);

		//Calculate components of BRDF
		vec3 F = F0 + (1 - F0)*pow((1 - dot(r, n)),5); 
		float G = dot(n,r)*dot(n,v);
		vec3 D = vec3(texture(specularCube, r)); //Sample specular cube
	
		//Part of the gamma correction
		D = vec3(pow(D.x, 2), pow(D.y, 2), pow(D.z, 2));

		vec3 BRDF = (F * G * D)/(4 * dot(n, r) * dot(n, v));
		vec4 specular = vec4(BRDF, 1.0f);

		vec4 diffuse = texture(diffuseCube, n); //Sample diffuse cube
		diffuse = vec4(pow(diffuse.x, 2), pow(diffuse.y, 2), pow(diffuse.z, 2), pow(diffuse.w, 2));
	
		vec4 temp = diffuse + specular;
		color = vec4(sqrt(temp.x), sqrt(temp.y), sqrt(temp.z), 1);
	}
	else if (reflection == 1)
	{
		vec3 v = normalize(fragPos - cameraPos);
		vec3 r = reflect(v, normalize(vertexNormal));
		color = texture(specularCube, r);
	}

}