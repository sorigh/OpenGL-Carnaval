#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec4 fragPosLightSpace;
in vec2 fTexCoords;

out vec4 fColor;

// Matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

// lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform int directionalLightEnabled;
struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

// fog
uniform int fog;
uniform vec4 fogColor; // Configurable fog color
uniform float fogDensity;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

// lighting components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

vec4 fPosEye;
vec3 normalEye;
vec3 viewDir;

void computeCommonValues() {
    fPosEye = view * model * vec4(fPosition, 1.0f);
    normalEye = normalize(normalMatrix * fNormal);
    viewDir = normalize(-fPosEye.xyz);
}

void computeDirLight() {
	if (directionalLightEnabled == 0){
		return;
	}
    vec3 lightDirN = normalize(vec3(view * vec4(lightDir, 0.0f)));
    // Ambient
    ambient = ambientStrength * lightColor;
    // Diffuse
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
    // Specular
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
}

vec3 computePointLight(PointLight light, vec3 fragPos, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0f);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f); // Use shininess if needed
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // Combine results
    vec3 ambient = light.ambient * attenuation;
    vec3 diffuse = light.diffuse * diff * attenuation;
    vec3 specular = light.specular * spec * attenuation;
    return (ambient + diffuse + specular);
}

float computeFog() {
    float fogDensity = 0.01f;
    float fragmentDistance = length(fPosEye);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
    return clamp(fogFactor, 0.0f, 1.0f);
}

void applyFog(inout vec4 fragColor) {
    vec4 fogColor = vec4(0.6f, 0.6f, 0.7f, 1.0f);
    float fogFactor = computeFog();
    fragColor = mix(fogColor, fragColor, fogFactor);
}

/*
float computeFog() {
    float fragmentDistance = length(fPosEye.xyz);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
    return clamp(fogFactor, 0.0f, 1.0f);
}
*/
float computeShadow() {	
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if (normalizedCoords.z > 1.0f)
        return 0.0f;

    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    float currentDepth = normalizedCoords.z;
    float bias = max(0.005f * (1.0f - dot(normalEye, vec3(0, 0, -1))), 0.001f);
    return currentDepth - bias > closestDepth ? 1.0f : 0.0f;
}

void main() {
    computeCommonValues();
    computeDirLight();
	
    vec3 totalPointLight = vec3(0.0);
	for(int i = 0; i < NR_POINT_LIGHTS; i++)
         totalPointLight += computePointLight(pointLights[i], fPosition, normalEye, viewDir);
	
    float shadow = computeShadow();

    vec3 texDiffuse = texture(diffuseTexture, fTexCoords).rgb;
    vec3 texSpecular = texture(specularTexture, fTexCoords).rgb;

    vec3 color = min((ambient + totalPointLight + diffuse * (1.0f - shadow)) * texDiffuse + specular * (1.0f - shadow) * texSpecular, 1.0f);
	
    if (fog == 1) {
        float fogFactor = computeFog();
        vec4 colorF = vec4(color, 1.0f);
        fColor = mix(fogColor, colorF, fogFactor);
    } else {
        fColor = vec4(color, 1.0f);
    }
}