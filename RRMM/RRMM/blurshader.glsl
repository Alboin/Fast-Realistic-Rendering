#version 330 core


in vec2 TexCoords;
out vec4 color;

uniform int blurRadius;
uniform int enableBlur;
uniform float nSamples;


uniform sampler2D screenTexture;



void main()
{
	vec4 colorVal =  texture(screenTexture, TexCoords);

	if(enableBlur == 0)
	{
		color = colorVal;
		return;
	}



	//const float gauss[] = { 0.00000067f,	0.00002292f,	0.00019117f,	0.00038771f,	0.00019117f,	0.00002292f,	0.00000067f,
	//					0.00002292f,	0.00078634f,	0.00655965f,	0.01330373f,	0.00655965f,	0.00078633f,	0.00002292f,
	//					0.00019117f,	0.00655965f,	0.05472157f,	0.11098164f,	0.05472157f,	0.00655965f,	0.00019117f,
	//					0.00038771f,	0.01330373f,	0.11098164f,	0.22508352f,	0.11098164f,	0.01330373f,	0.00038771f,
	//					0.00019117f,	0.00655965f,	0.05472157f,	0.11098164f,	0.05472157f,	0.00655965f,	0.00019117f,
	//					0.00002292f,	0.00078633f,	0.00655965f,	0.01330373f,	0.00655965f,	0.00078633f,	0.00002292f,
	//					0.00000067f,	0.00002292f,	0.00019117f,	0.00038771f,	0.00019117f,	0.00002292f,	0.00000067f };

	float radius = blurRadius;
	float increment = radius / nSamples;

	vec4 finalColor = vec4(0.0f);
	int totalSamples = 0;
	for(float i = 0; i < radius * 2 + 1; i += increment)
	{
		for(float j = 0; j < radius * 2 + 1; j += increment)
		{
			vec4 samplePoint = texture(screenTexture, TexCoords + (1.0f / 1000.0f) * vec2(i - (radius * 2 + 1) / 2.0f, j - (radius * 2 + 1) / 2.0f));
			//samplePoint *= gauss[i * 7 + j];
			if(samplePoint.x < 0 || samplePoint.y < 0 || samplePoint.z < 0 || samplePoint.w < 0)
				continue;
			finalColor += samplePoint;
			totalSamples++;
		}
	}
	finalColor = finalColor / totalSamples;


	color = finalColor;
}