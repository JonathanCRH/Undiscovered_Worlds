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
uniform vec4 bloomColor;
uniform float bDist; // Size of the bloom effect (0.0 - 1.0)
uniform float glareFactor; // Strength of the glare effect (0.0 - 1.0)
uniform float rayValue; // Overall strength of the ray effect
uniform int rayNo; // Number of rays

uniform vec4 colDiffuse;

vec4 texelColor = texture(texture0, fragTexCoord);
vec4 tranColor = sunColor; // Transparent colour.
//vec4 tintColor = vec4(0.4, 0.5, 1, 1); // Tint colour. (0.6, 0.6, 1, 1)

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    float thisGlareFactor = glareFactor * 0.5;

    vec4 tintColor = bloomColor;

    tintColor.r = min(tintColor.r, sunColor.r);
    tintColor.g = min(tintColor.g, sunColor.g);
    tintColor.b = min(tintColor.b, sunColor.b);

    float maxBloomSize = 16 * sunRadius; //320; // Maximum size of bloom
    vec2 size = textureSize(texture0, 0);

    float bloomDist = bDist * maxBloomSize;

    tranColor.a = 0; // Make this transparent.

    if (bloomDist == 0 && texelColor == vec4(1, 1, 1, 1)) // If we're on the sun
    {
        finalColor = sunColor;
    }
    else
    {
        finalColor = tranColor;

	if (bloomDist > 0)
	{
	    if (sunFactor >= 0) // If the sun is visible
	    {
		float thisBloomDist = min(bloomDist, bloomDist * (1 - sunFactor) + bloomDist * 0.2);

       	    	vec2 distToSun = fragTexCoord - sunPos;

	    	distToSun *= size;

	    	float lightDistance = sqrt(distToSun.x * distToSun.x + distToSun.y * distToSun.y);

	    	lightDistance -= sunRadius * (1 - sunFactor); // * 0.4

                float strength = 1 - (lightDistance / thisBloomDist);

                if (strength < 0)
                    strength = 0;

                if (strength > 1)
                    strength = 1;

                strength = strength * strength;

		float lightX = (sunPos.x - fragTexCoord.x) * size.x;
		float lightY = (sunPos.y - fragTexCoord.y) * size.y;

                float angle = atan(lightY, lightX);
                
                if (rayValue > 0) // Add a ray effect
                {
		    float thisRayValue = rayValue * 0.1;
	
		    float rayLimit = sunRadius * 0.25; // Rays will be reduced in strength this close to the sun.

                    float rayStrength = cos(angle * rayNo);

		    if (lightDistance < 0) // Reduce the ray effect close to the sun so it doesn't look odd.
		    {
			thisRayValue = 0;
		    }
		    else
		    {
		    	if (lightDistance < rayLimit)
		    	{
			    thisRayValue *= lightDistance / rayLimit;
		    	} 
		    }

                    rayStrength = strength * ((rayStrength + 1) / 2);

                    strength = rayStrength * thisRayValue + strength * (1 - thisRayValue);

		    if (thisGlareFactor > 0) // Add a glare effect
		    {
			float pixelX = 1 / size.x;
    			float pixelY = 1 / size.y;

			float thisY = fragTexCoord.y * size.y;

			vec2 sunPosPixels = sunPos * size;

			float thisSunRadius = sunRadius * (1 - sunFactor);

			if (thisY >= sunPosPixels.y - thisSunRadius && thisY <= sunPosPixels.y + thisSunRadius)
			{
			    float minGlareDist = thisSunRadius;
			    float extraGlareDist = size.x;

			    float dist = 0;
			
			    if (thisY < sunPosPixels.y)
			    {
				dist = sunPosPixels.y - thisY;
			    }

			    if (thisY > sunPosPixels.y)
			    {
				dist = thisY - sunPosPixels.y;
			    }

			    float proximity = thisSunRadius - dist;

		 	    float thisGlareDist = minGlareDist;
			    float extraFactor = min(1, proximity / thisSunRadius);
			    extraFactor = pow(extraFactor, 12);

			    thisGlareDist += extraGlareDist * extraFactor;

			    thisGlareDist -= thisSunRadius;

			    float glareStrength = 1 - (lightDistance / thisGlareDist);

			    if (glareStrength < 0)
			    	glareStrength = 0;

			    if (glareStrength > 1)
			    	glareStrength = 1;

			    glareStrength *= thisGlareFactor;

			    //strength = max(strength, glareStrength);
			    strength += glareStrength;

			    strength = min(strength, 1);
			}
		    }

		    float tintStart = 0.99;
		    float tintFull = 0.1; //0.1;

		    if (strength <= tintFull)
		    {
			finalColor = tintColor;
		    }

		    if (strength > tintFull && strength <= tintStart)
		    {
		    	float maxDiff = tintStart - tintFull;

			float thisDiff = strength - tintFull;

			float factor = thisDiff / maxDiff;

			finalColor = finalColor * factor + tintColor * (1 - factor);
		    }

		    //finalColor = finalColor * strength + tintColor * (1 - strength);
                
                    finalColor.a = strength;
		}
	    }
	}
    }


    finalColor = finalColor * colDiffuse;
}