#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in float sunRadius; // Its visual radius.
in vec2 sunPos; // This is a 2D coordinate.
in float sunFactor;  // If it's < 0, the sun is not visible. If it's 0, the sun is fully visible. If it's between 0 and 1, that is how close it is to being fully visible.

// Input uniform values
uniform sampler2D texture0;
uniform vec4 sunColor;
uniform float lensStrength;

uniform vec4 colDiffuse;

vec4 texelColor = texture(texture0, fragTexCoord);
vec4 tranColor = sunColor; // Transparent colour.

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    vec2 size = textureSize(texture0, 0);

    tranColor.a = 0;

    finalColor = tranColor;

    float thisLensStrength = lensStrength * 0.65;

    if (thisLensStrength > 0 && sunFactor >= 0)
    {
	//float thisSunRadius = sunRadius * (1 - sunFactor);

	vec2 sunOffset = sunPos - vec2(0.5, 0.5);

	vec4 blobColor = vec4(0);

	for (int n = 0; n < 6; n++) // Three blobs, one in red, one in green, and one in blue.
	{
	    float magnitude = 0; // This is how much this blob will be offset.
	    float thisOverallStrength = 1; // Overall strength of this blob.
	    float blobRadius = sunRadius * 0.5;
	    float blobBloomDist = sunRadius * 0.75; // Distance of bloom from the blobs.

	    if (n == 0) // Red blob
	    {
		magnitude = -1.2;
	    }

	    if (n == 1) // Green blob
	    {
		magnitude = -1.23;
	    }

	    if (n == 2) // Blue blob
	    {
		magnitude = -1.26;
	    }

	    if (n == 3)
	    {
		magnitude = -0.4;
		blobRadius = sunRadius * 2;
		//thisOverallStrength = 0.25;
	    }

	    if (n == 4)
	    {
		magnitude = -0.45;
		blobRadius = sunRadius * 2;
		//thisOverallStrength = 0.25;
	    }

	    if (n == 5)
	    {
		magnitude = -0.5;
		blobRadius = sunRadius * 2;
		//thisOverallStrength = 0.25;
	    }

	    vec2 blobCentrePos = vec2(0.5) + sunOffset * magnitude;

	    float thisBlobBloomDist = min(blobBloomDist, blobBloomDist * (1 - sunFactor) + blobBloomDist * 0.2);

	    vec2 distToBlob = fragTexCoord - blobCentrePos;

	    distToBlob *= size;

	    float lightDistance = sqrt(distToBlob.x * distToBlob.x + distToBlob.y * distToBlob.y);

	    lightDistance -= blobRadius * (1 - sunFactor);

	    float thisBlobStrength = 1 - (lightDistance / thisBlobBloomDist);
	    thisBlobStrength *= thisOverallStrength;

	    if (thisBlobStrength > 0)
	    {
		vec4 thisBlobColor = vec4(0);

		if (n == 0 || n == 3)
		    thisBlobColor.r = sunColor.r;

		if (n == 1 || n == 4)
		    thisBlobColor.g = sunColor.g;

		if (n == 2 || n == 5)
		    thisBlobColor.b = sunColor.b;

		if (finalColor == tranColor)
		    finalColor = vec4(0);

		thisBlobColor.a = thisBlobStrength;

		blobColor.r = min(1, blobColor.r + thisBlobColor.r * thisBlobStrength);
		blobColor.g = min(1, blobColor.g + thisBlobColor.g * thisBlobStrength);
		blobColor.b = min(1, blobColor.b + thisBlobColor.b * thisBlobStrength);
		blobColor.a = min(1, blobColor.a + thisBlobColor.a * thisBlobStrength);
	    }
	}

	blobColor *= thisLensStrength;
	blobColor *= (1 - sunFactor); // Make it fainter if the sun is only partially seen.

	finalColor.r = max(finalColor.r, blobColor.r);
	finalColor.g = max(finalColor.g, blobColor.g);
	finalColor.b = max(finalColor.b, blobColor.b);
	finalColor.a = max(finalColor.a, blobColor.a);
    }

    finalColor = finalColor * colDiffuse;
}