#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec3 fragPosition;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;

// Input uniform values
uniform sampler2D texture0; // Diffuse
uniform sampler2D texture1; // r = specular; g = roughness; b = cloud;
uniform sampler2D texture2; // Normal (this never changes, so no need for an old version too)
uniform sampler2D olddiffuse; // Using MATERIAL_MAP_OCCLUSION
uniform sampler2D oldspecular; // Using MATERIAL_MAP_EMISSION
uniform vec4 colDiffuse;
uniform mat4 matModel;
uniform vec4 ambient;
uniform vec3 viewPos;
uniform vec4 atmosCentreColor;
uniform vec4 duskColor;
uniform float bumpiness;
uniform float haze;
uniform float specularOverall;
uniform float cloudCover;
uniform float gamma;
uniform float showCurrentMonth;
uniform vec4 lightCol;
uniform vec3 lightPos;
uniform vec3 lightTarget;

// Output fragment color
out vec4 finalColor;

void main()
{
    //float bumpiness = 0.2; // 0 = no bump mapping. 1 = full bump mapping.
    float hazePoint = 0.2; // If haze is lower than this, atmosphere effects will be reduced.
    float duskPoint = 0.3; // The bigger this is, the more dusk colour there will be.
    float cloudPointb = 1.0 - cloudCover; // Above this point, there are opaque clouds.
    float cloudPointa = max(cloudPointb - 0.1, 0.0); // Above this point, there are translucent clouds.
    float cloudDiff = cloudPointb - cloudPointa;
    float cloudSpec = 0.6; // How shiny the clouds are.
    float cloudRough = 0.2; //0.7; // How rough the clouds are.

    vec4 texelColor = texture(texture0, fragTexCoord); // Diffuse map
    vec4 normalMap = texture(texture2, fragTexCoord);   // Normal map
    float specularMap = texture(texture1, fragTexCoord).r; // Specular map
    float roughness = texture(texture1, fragTexCoord).g;   // Roughness map
    float cloudMap = texture(texture1, fragTexCoord).b;   // Cloud map

    vec4 cloudColor = vec4(1.0);
    vec3 lightDot = vec3(0.0);
    vec3 flatNormal = normalize(fragNormal); // The normal ignoring the normal map.
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);
    bool cloud = false; // Whether this is cloud or not.
    float thisGamma = gamma;

    // If we're transitioning between months, change some of the values.

    if (showCurrentMonth < 1.0)
    {
        vec4 texelColorOld = texture(olddiffuse, fragTexCoord);
        float specularMapOld = texture(oldspecular, fragTexCoord).r;
        float roughnessOld = texture(oldspecular, fragTexCoord).g;
        float cloudMapOld = texture(oldspecular, fragTexCoord).b;

        texelColor = texelColor * showCurrentMonth + texelColorOld * (1.0 - showCurrentMonth);
        specularMap = specularMap * showCurrentMonth + specularMapOld * (1.0 - showCurrentMonth);
	roughness = roughness * showCurrentMonth + roughnessOld * (1.0 - showCurrentMonth);
        cloudMap = cloudMap * showCurrentMonth + cloudMapOld * (1.0 - showCurrentMonth);
    }

    // Sort out normals.

    mat3 TBN = transpose(mat3(fragTangent, fragBitangent, fragNormal));

    vec3 normal = normalMap.rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(normal * TBN);

    // Work out a few more variables.

    float thisBumpiness = bumpiness * (1.0 - haze) * 0.8; // The hazier it is, the less we can see bumpiness of the land.
    normal = normal * thisBumpiness + flatNormal * (1.0 - thisBumpiness);

    vec4 atmosEdgeColor = atmosCentreColor * 1.2; // Colour of the atmosphere at the limb of the planet.

    texelColor = texelColor * (1.0 - haze) + atmosCentreColor * haze; // Add some atmospheric haze everywhere.
    vec4 texelColorNoCloud = texelColor;

    if (cloudMap >= cloudPointa) // If it's cloudy here, change the colour to clouds.
    {
        cloud = true;

	if (cloudPointb == 0.0 || cloudMap >= cloudPointb) // Opaque clouds.
	{
        	texelColor = cloudColor;
        	normal = flatNormal; // No bumpiness on the clouds.
		roughness = cloudRough;
	}
	else // Transparent clouds.
	{
		float cloudFactor = cloudMap - cloudPointa;
		cloudFactor = cloudFactor / cloudDiff;

		texelColor = cloudColor * cloudFactor + texelColor * (1.0 - cloudFactor);
		normal = flatNormal * cloudFactor + normal * (1.0 - cloudFactor);
		//roughness = (roughness + (cloudRough * cloudFactor + roughness * (1.0 - cloudFactor)) * 2.0) / 3.0;
		roughness = cloudRough * cloudFactor + roughness * (1.0 - cloudFactor);
	}
    }

    vec3 lightDir = -normalize(lightTarget - lightPos);

    float flatNdotLmin = dot(flatNormal, lightDir);
    float flatNdotL = max(flatNdotLmin, 0.0);

    if (flatNdotL == 0.0) // If this would be in shadow, flatten the bumps.
    {
	float flatMult = min(flatNdotLmin * -80.0, 10.0);
	float bumpMult = 10.0 - flatMult;
	normal = (normal * bumpMult + flatNormal * flatMult) / 10.0;
    }

    float NdotL = max(dot(normal, lightDir), 0.0);
    lightDot += lightCol.rgb * NdotL;

    // Specular lighting.

    float specFactor = specularMap;

    vec3 reflectDir = reflect(-lightDir, normal);  
    float specPow = max(36.0 * roughness, 2.0);

    float specadd = pow(max(dot(viewDir, reflectDir), 0.0), specPow); // 24.0
    float hazeSpecularMult = min(haze, 0.75);
    vec3 addToSpecular = specadd * lightCol.rgb * max(specFactor, hazeSpecularMult * 0.65);

    if (cloud == true)
    {
	if (cloudPointb == 0.0 || cloudMap >= cloudPointb) // Opaque clouds.
	{
        	specFactor = cloudSpec;

    		vec3 reflectDir = reflect(-lightDir, normal);  
    		float specPow = max(36.0 * roughness, 2.0);

    		float specadd = pow(max(dot(viewDir, reflectDir), 0.0), specPow); // 24.0
    		float hazeSpecularMult = min(haze, 0.75);
    		addToSpecular = specadd * lightCol.rgb * max(specFactor, hazeSpecularMult * 0.65);
	}
	else // Transparent clouds.
	{
		float cloudFactor = cloudMap - cloudPointa;
		cloudFactor = cloudFactor / cloudDiff;

		specFactor = cloudSpec * cloudFactor + specFactor * (1.0 - cloudFactor);

    		vec3 reflectDir = reflect(-lightDir, normal);  
    		float specPow = max(36.0 * roughness, 2.0);

    		float specadd = pow(max(dot(viewDir, reflectDir), 0.0), specPow); // 24.0
    		float hazeSpecularMult = min(haze, 0.75);
    		vec3 cloudAddToSpecular = specadd * lightCol.rgb * max(specFactor, hazeSpecularMult * 0.65);

		if (cloudAddToSpecular.r > addToSpecular.r)
			addToSpecular.r = cloudAddToSpecular.r;

		if (cloudAddToSpecular.g > addToSpecular.g)
			addToSpecular.g = cloudAddToSpecular.g;

		if (cloudAddToSpecular.b > addToSpecular.b)
			addToSpecular.b = cloudAddToSpecular.b;

	}
    }

    specular += addToSpecular;

    specular *= specularOverall * 2.0;

    // Add dusk colour effect.

    if (flatNdotLmin < duskPoint)
    {
	float dusk = min(pow(1.0 - (flatNdotLmin / duskPoint), 2.0), 0.6);

	if (haze < hazePoint) // If the atmosphere is thin, this effect is reduced.
		dusk = dusk * (haze / hazePoint);

	texelColor = texelColor * (1.0 - dusk) + duskColor * dusk;

	thisGamma *= (1 - dusk);
    }

    // Apply all of that to the fragment colour.

    finalColor = (texelColor * (colDiffuse * vec4(lightDot, 1.0)));
    finalColor.rgb += specular;
    finalColor += texelColor * (ambient / 5.0) * colDiffuse;

    // Now do the atmosphere.

    float atmosPoint = 0.15; // The bigger this is, the more the atmosphere will extend into space.

    vec4 finalAtmosColor = vec4(0.0);

    float viewDot = dot(flatNormal, viewDir);

    if (viewDot < atmosPoint) // Adding atmosphere at the limb of the planet.
    {
	finalAtmosColor = atmosEdgeColor;

	vec3 viewToLight = normalize(viewPos - lightPos); // Direction from the camera to the light.
	float lightCloseness = dot(viewDir, viewToLight); // How closely the direction to the fragment matches the direction to the light.

	float brightnessFromEclipse = 0.0; // This will multiply brightness if the sun is visually close to this point of the atmosphere.

	if (lightCloseness > 0.0)
	{
		float distance = length(viewPos); // Distance from the camera to the planet.
		float eclipsePower = distance * 6.0; // 2.0;
		brightnessFromEclipse = pow(lightCloseness, eclipsePower) * 1.5;

		float duskStrength = min(brightnessFromEclipse * 0.5, 1.0);
		finalAtmosColor = duskColor * duskStrength + finalAtmosColor * (1.0 - duskStrength);
	}

	float brightnessFromElev = max((1.0 / atmosPoint) * viewDot, brightnessFromEclipse * 0.7);

	if (haze < hazePoint)
		brightnessFromElev *= haze / hazePoint;

	float brightnessFromLight = min(flatNdotL * 2.0, 1.0) + brightnessFromEclipse;

	finalColor = finalAtmosColor * lightCol * brightnessFromElev * brightnessFromLight;
    }
    else // Add atmosphere effects to the main body of the planet.
    {
	finalAtmosColor = atmosCentreColor;

	finalAtmosColor = finalAtmosColor * (colDiffuse * vec4(lightDot, 1.0));

	float opacity = min(pow(1.0 - viewDot + atmosPoint + 0.1, 4.0), 1.0); // + 0.1, 4.0 // // The higher this is, the more opaque the atmosphere.

	if (haze < hazePoint)
		opacity *= haze / hazePoint;

	if (cloud) // Less haze if there are clouds, as they should be above much of it.
	{
		if (cloudPointb == 0.0 || cloudMap >= cloudPointb) // Opaque clouds.
		{
     		   	opacity *= 0.5;
		}
		else // Transparent clouds.
		{
			float cloudFactor = cloudMap - cloudPointa;
			cloudFactor = cloudFactor / cloudDiff;

			opacity *= 1.0 - (cloudFactor * 0.5);
		}
	}

	finalColor = finalColor * (1.0 - opacity) + finalAtmosColor * opacity;
    }

    // Add ambient light to dark areas if necessary.

    if (finalColor.r < texelColor.r * (ambient.r / 5.0))
        finalColor.r = texelColor.r * (ambient.r / 5.0);

    if (finalColor.g < texelColor.g * (ambient.g / 5.0))
        finalColor.g = texelColor.g * (ambient.g / 5.0);

    if (finalColor.b < texelColor.b * (ambient.b / 5.0))
        finalColor.b = texelColor.b * (ambient.b / 5.0);

    if (gamma > 0)
    {
    	vec4 gammaColor = pow(finalColor, vec4(1.0/2.2));

	finalColor = gammaColor * thisGamma + finalColor * (1 - thisGamma);
    }
}
