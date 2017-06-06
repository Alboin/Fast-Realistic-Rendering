#version 330 core

in vec2 TexCoords;
out vec4 color;

uniform int radiusPixels;
uniform float nSamples;
uniform int randomize;
uniform vec3 cameraPos;
uniform mat4 viewMatrix;
uniform mat4 perspective;

uniform sampler2D screenTexture;

mat4 rotationMatrix(vec3 axis, float angle);
highp float rand(vec2 co);
//vec3 viewSpacePos(vec2 texCoords, float depth);
//vec3 textureSpacePos(vec3 viewPos);

void main()
{ 
	// Calculate normal and depth from texture.
	vec4 colorVal = texture(screenTexture, TexCoords);
	float depth = colorVal.w;
	vec3 normal = vec3(colorVal);

	////DEBUG
	//color = vec4(vec3(depth), 1.0f);
	//return;
	//color = colorVal;
	//return;
	
	// Set the background color to white.
	if(depth > 0.99f)
	{
		color = vec4(1.0f);
		return;
	}
	
	// Initiate sampling vectors.
	vec2[4] sampleVectors;
	sampleVectors[0] = normalize(vec2(0,1));
	sampleVectors[1] = normalize(vec2(1,0));
	sampleVectors[2] = normalize(vec2(0,-1));
	sampleVectors[3] = normalize(vec2(-1,0));

	// Set the radius and sample-length depending on the resolution of the image.
	float windowResolution = 1000.0f;
	float radius = (radiusPixels / windowResolution) / (depth * (5.0f - 0.1f));
	float sampleLength = radius / nSamples;

	// Rotate sampling-vectors around z-axis equally by a random amount.
	if(randomize == 1)
	{
		float randomAngle = rand(TexCoords) * 3.14f;
		for(int i = 0; i < 4; i++)
			sampleVectors[i] = vec2(vec4(sampleVectors[i], 0, 0) * rotationMatrix(vec3(0,0,1), randomAngle));
	}

	// Set the startpoint, the point of our current fragment.
	vec3 startPoint = vec3(TexCoords, 1.0f - depth);
	float ambientOcclusion = 0.0f;


	// Loop through the sample vectors.
	for(int i = 0; i < 4; i++)
	{
		//vec3 highestPointVec = vec3(TexCoords + sampleVectors[i] * radius, 1.0f - texture(screenTexture, TexCoords + sampleVectors[i] * radius).w);
		//vec3 highestPointVec = vec3(TexCoords + sampleVectors[i] * sampleLength, 1.0f - texture(screenTexture, TexCoords + sampleVectors[i] * sampleLength).w);
		vec3 highestPointVec = startPoint;
		float sampleWeight = 1;
		
		// Walk along the vector.
		for(float j = sampleLength; j < radius; j += sampleLength)
		{
			vec2 texturePos = TexCoords + sampleVectors[i] * j;
			vec4 samplePoint = texture(screenTexture, texturePos);
			
			// If the sampled point is at a lower depth than the highest point, set it as new highest point.
			if(1.0f - samplePoint.w > highestPointVec.z && abs(1.0f - samplePoint.w - highestPointVec.z) < 0.1f)
			{
				highestPointVec = vec3(texturePos, 1.0f - samplePoint.w);
				sampleWeight = max(0.0f, 1.0f - (j / radius));

				vec3 horizontalVec = highestPointVec - startPoint;
				float horizontalAngle = acos(dot(horizontalVec, vec3(horizontalVec.xy, 0.0f)));

				////DEBUG
				//color = vec4(normal, 1.0f);
				//return;
			}	
		}

		// Calculate the horizontal angle.
		vec3 horizontalVec = highestPointVec - startPoint;
		//// If no higher point was found, set the 
		//if(distance(horizontalVec, startPoint) < 0.001f)
		//	horizontalVec = startPoint;

		//float horizontalAngle = atan(length(horizontalVec.z) / length(horizontalVec.xy));
		float horizontalAngle = acos(dot(horizontalVec, vec3(horizontalVec.xy, 0.0f)));
		// The horizontal angle should be signed.
		if(horizontalVec.z < 0)
			horizontalAngle = -horizontalAngle;

		// Axises that represent the screens xyz in world coordinates.
		vec3 yAxis = vec3(0,1,0);
		vec3 zAxis = normalize(cameraPos);
		vec3 xAxis = normalize(cross(yAxis, zAxis));

		// Get the normal and cameraPos in screenspace.
		vec3 normalInScreenspace = vec3(dot(xAxis, normal), dot(yAxis, normal), dot(zAxis, normal));
		vec3 cameraInScreenspace = vec3(dot(xAxis, cameraPos), dot(yAxis, cameraPos), dot(zAxis, cameraPos));

		vec3 temp = vec3(sampleVectors[i], 0) * dot(normalInScreenspace, vec3(sampleVectors[i], 0));
		vec3 difference = sampleVectors[i] - vec3(normalInScreenspace.xy, 0);
		vec3 alignedNormal = normalize(normalInScreenspace + difference);

		//vec3 alignedNormal = normalize(normal + a);
		float tangentialAngle = acos(dot(alignedNormal, normalize(cameraInScreenspace)));

		
		ambientOcclusion += (1.0f / (2*3.14)) * (sin(horizontalAngle) - sin(tangentialAngle)) * sampleWeight;

	}

	ambientOcclusion = ambientOcclusion;

	color = vec4(vec3(ambientOcclusion * 2 + 0.3f), 1.0f);
	
	//DEBUG
	if(ambientOcclusion < 0)
		color = vec4(vec3(1.0f, 0.0f, 0.0f), 1.0f);
}

// Function for creating a rotation matrix for rotation around a given axis and angle.
mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

// Function for generating a random number depending on texture coordinates.
highp float rand(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

//vec3 viewSpacePos(vec2 texCoords, float depth)
//{
//	//vec3 temp = normalize(cameraPos) * (5.0f - 0.1f);
//	//vec3 z = temp * depth;

//	//return vec3((texCoords.x * 2.0f - 1.0f) / perspective[0][0], (texCoords.y * 2.0f - 1.0f) / perspective[1][1], 1.0f) * depth;

//	vec3 temp = normalize(cameraPos) * (5.0f - 0.1f);
//	return temp * depth;
		
//	//// Get x/w and y/w from the viewport position
//	//float x = texCoords.x * 2 - 1;
//	//float y = (1 - texCoords.y) * 2 - 1;
//	//vec4 vProjectedPos = vec4(x, y, depth, 1.0f);
//	//// Transform by the inverse projection matrix
//	//vec4 vPositionVS = vProjectedPos * inverse(perspective);  
//	//// Divide by w to get the view-space position
//	//return vPositionVS.xyz / vPositionVS.w;  
//}

//vec3 textureSpacePos(vec3 viewPos)
//{
//	vec4 temp = vec4(viewPos, 1.0f) * perspective;
//	// Convert from screenspace to texture-space
//	return vec3(temp.x + 1.0f, temp.y + 1.0f, temp.z + 1.0f) / 2.0f;
//}