#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
//in vec4 fragColor;
in vec3 fragNormal;
in vec4 fragTangent;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec4 colDiffuse;
uniform mat4 matModel;
//uniform vec3 viewPos;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);  // Diffuse map
    vec4 specularMap = texture(texture1, fragTexCoord); // Specular map
    vec4 normalMap = texture(texture2, fragTexCoord);   // Normal map (not used yet)
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    // NOTE: Implement here your fragment shader code

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].target - lights[i].position);
            }

            if (lights[i].type == LIGHT_POINT)
            {
                light = normalize(lights[i].position - fragPosition);
            }

            float NdotL = max(dot(normal, light), 0.0);
            lightDot += lights[i].color.rgb * NdotL;

            // Specular lighting.

            vec3 reflectDir = reflect(-light, normal);  
            float specadd = pow(max(dot(viewDir, reflectDir), 0.0), 24.0);
            specular += specadd * lights[i].color.rgb * specularMap.r;  
        }
    }

    finalColor = (texelColor * (colDiffuse * vec4(lightDot, 1.0)));
    finalColor += specular;
    finalColor += texelColor*(ambient / 10.0)*colDiffuse;

    // Gamma correction (effectively atmospheric haze)
    finalColor = pow(finalColor, vec4(1.0 / 2.2));
}
