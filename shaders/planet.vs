#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexTangent;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragTangent;
out vec3 fragBitangent;

// NOTE: Add here your custom variables

void main()
{
    vec3 vertexBinormal = cross(vertexNormal, vertexTangent.rgb);

    mat3 normalMatrix = transpose(inverse(mat3(matModel)));

    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;

    fragNormal = normalize(normalMatrix * vertexNormal); 
    fragTangent = normalize(normalMatrix * vertexTangent.rgb);
    fragTangent = normalize(fragTangent - dot(fragTangent, fragNormal) * fragNormal);
    fragBitangent = cross(fragNormal, fragTangent);

    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
