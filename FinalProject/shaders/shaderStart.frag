#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 4.0f;
vec3 diffuse;
vec3 specular;
float specularStrength = 4.0f;
float shininess = 64.0f;

//For point light
float constant = 1.0f;
float linear = 0.09f;
float quadratic = 0.032f;

uniform mat4 view;

//Point light
uniform vec3 pointLightPosition;

float ambientStrengthDirect = 0.2f;
float specularStrengthDirect = 0.5f;
float shininessDirect = 32.0f;



float computeShadow()
{
	//perform perspective divide
	vec3 normalizedCoords= fragPosLightSpace.xyz / fragPosLightSpace.w;

	//tranform from [-1,1] range to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	//get closest depth value from lights perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	//get depth of current fragment from lights perspective
	float currentDepth = normalizedCoords.z;

	//if the current fragments depth is greater than the value in the depth map, the current fragment is in shadow 
	//else it is illuminated
	//float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
	float bias = 0.005f;
	float shadow; 
	if(currentDepth - bias > closestDepth)
		shadow = 1.0;
	else
		shadow = 0.0;

	if (normalizedCoords.z > 1.0f)
		return 0.0f;

	return shadow;
}

vec3 computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrengthDirect * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor * 0.5;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininessDirect);
	specular = specularStrengthDirect * specCoeff * lightColor;

	return (ambient + diffuse + specular);
}

vec3 computePointLightComponents() {
	vec4 lightPosEye = view * vec4(pointLightPosition, 1.0);
	vec3 cameraPosEye = vec3(0.0f);
	vec3 normalEye = normalize(fNormal);
	vec3 lightDirN = normalize(lightPosEye.xyz - fPosEye.xyz);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	vec3 ambient = ambient * lightColor;
	vec3 diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	//vec3 halfVector = normalize(lightDirN + viewDirN);
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, reflection), 0.0f), shininess);
	vec3 specular = specularStrength * specCoeff * lightColor;
	float distance = length(lightPosEye.xyz - fPosEye.xyz);
	float att = 1.0f / (constant + linear * distance + quadratic * distance * distance);
	return (ambient + diffuse + specular) * att;
	
}

float computeFog()
{
	float fogDensity = 0.007f;
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
	return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
	vec3 light = computeLightComponents();
	float shadow = computeShadow();
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	//vec3 light = computePointLightComponents();
	light += computePointLightComponents();
	
	vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);
	//vec3 color = min((ambient + diffuse) + specular, 1.0f);
    float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	//fColor = vec4(color, 1.0f);
	vec4 temp = vec4(color, 1.0f);
	//fColor = mix(fogColor, temp ,fogFactor);
	fColor = mix(fogColor, min(temp * vec4(light, 1.0f), 1.0f), fogFactor);
}
