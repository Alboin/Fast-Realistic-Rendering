#version 330 core

in vec3 vertexColor;
in vec3 vertexNormal;
in vec3 fragPos;

out vec4 color;

uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform float fresnel;

uniform samplerCube diffuseCube;
uniform samplerCube specularCube;

void main()
{
	//Initiate variables
	vec3 v = normalize(fragPos - cameraPos);
	vec3 r = reflect(v, normalize(vertexNormal));
	vec3 n = vertexNormal;
	vec3 F0 = vec3(fresnel, fresnel, fresnel);

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