#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0; // Stars
uniform sampler2D haze; // Using MATERIAL_MAP_OCCLUSION
uniform vec4 colDiffuse;
uniform float starBrightness;
uniform float starColor;
uniform float hazeBrightness;
uniform float nebulaBrightness;
uniform vec4 hazeColor1;
uniform vec4 hazeColor2;
uniform vec4 nebulaColor1;
uniform vec4 nebulaColor2;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    finalColor = vec4(0, 0, 0, 1);

    vec4 hazeInfo = texture(haze, fragTexCoord);       
    float hazeStrength = hazeInfo.r;
    float hazeColor = hazeInfo.g;
    float nebulaStrength = hazeInfo.b;
    float nebulaColor = hazeInfo.a;

    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    // First, get the haze colour.

    vec4 currentHazeColor = hazeColor1 * hazeColor + hazeColor2 * (1 - hazeColor);

    currentHazeColor *= hazeStrength;
    currentHazeColor *= hazeBrightness;

    // Now, get the nebula colour.

    vec4 currentNebulaColor = nebulaColor1 * nebulaColor + nebulaColor2 * (1 - nebulaColor);

    currentNebulaColor *= nebulaStrength;
    currentNebulaColor *= nebulaBrightness;

    // Add them together.

    currentHazeColor += currentNebulaColor;

    // Now, get the star colour.

    vec4 currentStarColor = texelColor;
    
    float thisBright = max(currentStarColor.r, max(currentStarColor.g, currentStarColor.b));

    vec4 whiteStarColor = vec4(thisBright, thisBright, thisBright, 1);

    currentStarColor = currentStarColor * starColor + whiteStarColor * (1 - starColor);
    
    if (thisBright <= 1 - starBrightness)
    {
        finalColor = currentHazeColor;
    }
    else
    {
        if (thisBright > max(currentHazeColor.r, (currentHazeColor.g, currentHazeColor.b)))
        {
            finalColor = currentStarColor;
        }
        else
        {
            finalColor = currentHazeColor;
        }
    }

    finalColor = finalColor * colDiffuse;
}

