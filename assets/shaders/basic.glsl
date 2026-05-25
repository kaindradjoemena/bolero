#TYPE VERTEX
#version 460 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texCoords;
layout (location = 3) in vec3 a_tangent;
layout (location = 4) in vec3 a_bitangent;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projMat;
uniform mat3 u_normMat;

out vec3 v_fragPos;
out vec3 v_normal;
out vec2 v_texCoords;

void main()
{
    v_fragPos = vec3(u_modelMat * vec4(a_position, 1.0));
    v_normal = u_normMat * a_normal;
    v_texCoords = a_texCoords;

    gl_Position = u_projMat * u_viewMat * u_modelMat * vec4(a_position, 1.0);
}


#TYPE FRAGMENT
#version 460 core
in vec3 v_fragPos;
in vec3 v_normal;
in vec2 v_texCoords;

out vec4 frag_color;

uniform vec3  u_lightCol;
uniform float u_lightPow;
uniform vec3  u_lightPos;
uniform float u_lightRange;
uniform vec3  u_camPos;

uniform sampler2D u_albedoMap;     uniform bool u_hasAlbedoMap;     uniform vec3  u_albedoFactor;
uniform sampler2D u_normalMap;	   uniform bool u_hasNormalMap;
uniform sampler2D u_metallicMap;   uniform bool u_hasMetallicMap;   uniform float u_metallicFactor;
uniform sampler2D u_roughnessMap;  uniform bool u_hasRoughnessMap;  uniform float u_roughnessFactor;
uniform sampler2D u_aoMap;		   uniform bool u_hasAOMap;		    uniform float u_AOFactor;

void main()
{
    vec3 normal;
    if (u_hasNormalMap)
	{
        normal = normalize(texture(u_normalMap, v_texCoords).rgb * 2.0 - 1.0);
    }
	else
	{
        normal = normalize(v_normal);
    }

    vec3 lightVec = u_lightPos - v_fragPos;
    float distanceSq = dot(lightVec, lightVec);
    vec3 lightDir = normalize(lightVec);

    float attenuation = u_lightPow / max(distanceSq, 0.0001f);
    float rangeSq = u_lightRange * u_lightRange;
    float distSqOverRangeSq = distanceSq / rangeSq;
    float windowing = clamp(1.0 - (distSqOverRangeSq * distSqOverRangeSq), 0.0, 1.0);
    windowing = windowing * windowing; 
    attenuation *= windowing;

    vec3 baseColor = u_hasAlbedoMap ? texture(u_albedoMap, v_texCoords).rgb : u_albedoFactor;
    vec3 diffuseColor = baseColor * max(dot(lightDir, normal), 0.0);

    float roughness = u_hasRoughnessMap ? texture(u_roughnessMap, v_texCoords).g : u_roughnessFactor;
    
    float shininess = max((1.0 - roughness) * 2048.0, 0.0001);

    vec3 viewDir = normalize(u_camPos - v_fragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(halfDir, normal), 0.0);
    float specular = pow(specAngle, shininess);

    vec3 ambientColor = vec3(0.03) * baseColor;
    vec3 specColor = vec3(1.0);
    float screenGamma = 2.2;

    vec3 colorLinear = ambientColor +
                       (diffuseColor * u_lightCol * attenuation) +
                       (specColor * specular * u_lightCol * attenuation);
                    
    // Gamma correction
    frag_color = vec4(pow(colorLinear, vec3(1.0 / screenGamma)), 1.0);
}