#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform sampler2D texture0;
uniform mat4 mvp;
uniform vec2 sunCentre; // The actual centre of the sun, whether or not it's visible.

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec3 fragPosition;
out vec4 fragColor;
out float sunRadius;
out vec2 sunPos;
out float sunFactor;

// NOTE: Add here your custom variables

void main()
{
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);

    // Now work out where the sun is!
    // We do this in the vertex shader to save time.
    // The vertex shader is called much less frequently than the fragment shader,
    // but this calculation is the same for every fragment, so best to do it here and pass on the result.

    vec2 size = textureSize(texture0, 0);
    float pixelX = 1 / size.x;
    float pixelY = 1 / size.y;

    // First find the radius of the sun as it appears on the screen.

    float searchDist = 100;
    vec2 searchDistPixels = vec2(searchDist * pixelX, searchDist * pixelY);

    sunRadius = 20;

    float i = -searchDist;
    float dist = 0;

    for (float x = sunCentre.x - searchDistPixels.x; x <= sunCentre.x + searchDistPixels.x; x += pixelX)
    {
	i++;

	float j = -searchDist;

	for (float y = sunCentre.y - searchDistPixels.y; y <= sunCentre.y + searchDistPixels.y; y += pixelY)
	{
	    j++;

	    if (texelFetch(texture0, ivec2(vec2(x, y) * size), 0) == vec4(1, 1, 1, 1))
	    {
		float thisDist = i * i + j * j;

		if (thisDist > dist)
		{
		    dist = thisDist;
		}
	    }		
	}
    }

    if (dist > 0) // We found one!
    {
	sunRadius = sqrt(dist);
    }

    // Now work out how much of the sun is visible, and where its effective centre is.

    vec2 sunRadiusPixels = vec2(sunRadius * pixelX, sunRadius * pixelY);

    sunPos = sunCentre;

    sunFactor = -1; // Negative amount means the sun is not visible at all.
            
    if (sunCentre.x >= -sunRadiusPixels.x && sunCentre.x <= 1 + sunRadiusPixels.x && sunCentre.y >= -sunRadiusPixels.y  && sunCentre.y <= 1 + sunRadiusPixels.y) // If the sun is supposedly visible
    {
	if (texelFetch(texture0, ivec2(sunCentre * size), 0) == vec4(1, 1, 1, 1)) // The centre of the actual sun is visible.
	{
	    sunFactor = 0; // To show that it is visible.
	}
	else // We need to find the point of the visible portion of the sun that's closest to its centre, if there is one.
	{
	    float sunCentreDist = 10000000;

	    float iLeft = -sunRadiusPixels.x;
	    float iRight = sunRadiusPixels.x;
	    float jUp = -sunRadiusPixels.y;
	    float jDown = sunRadiusPixels.y;

	    for (float i = iLeft; i <= iRight; i += pixelX)
	    {
		for (float j = jUp; j <= jDown; j += pixelY)
		{
		    if (texelFetch(texture0, ivec2((sunCentre + vec2(i, j)) * size), 0) == vec4(1, 1, 1, 1))
		    {
			float ii = i * size.x;
			float jj = j * size.y;

			float thisDist = ii * ii + jj * jj;

			if (thisDist < sunCentreDist)
			{
			    sunCentreDist = thisDist;
			    sunPos.x = sunCentre.x + i;
			    sunPos.y = sunCentre.y + j;
			}
		    }
		}
	    }

	    if (sunCentreDist != 10000000) // We found one!
	    {
		sunFactor = sqrt(sunCentreDist); 
		sunFactor = sunFactor / sunRadius;
	    }
	}                   
    }
}