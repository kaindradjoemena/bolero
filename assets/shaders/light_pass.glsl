// light_pass.glsl

#TYPE VERTEX
#version 460 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoords;

// ===== Light Structs =====
struct PointLightData
{
    vec4 positionAndRange; 
    vec4 colorAndPower;    
};
struct DirLightData
{
    vec4 directionAndPower;
    vec4 color;
};
struct SpotLightData
{
    vec4 positionAndLength;
    vec4 directionAndInner;
    vec4 colorAndOuter;
    vec4 powerAndPadding;
};

// ===== Instance Mats =====
struct InstanceData
{
    mat4 model;
    mat4 normal;
};


layout(std140, binding = 0) uniform CameraBuffer
{
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProj;
    vec4 u_CameraPosAndTime;
};
layout(std430, binding = 1) readonly buffer InstanceBuffer
{
    InstanceData u_Instances[];
};
layout(std430, binding = 2) readonly buffer LightBuffer
{
    uint u_DirCount;
    uint u_PointCount;
    uint u_SpotCount;
    uint u_Padding;
    
    DirLightData   u_DirLights[16];
    PointLightData u_PointLights[1024];
    SpotLightData  u_SpotLights[512];
};

uniform uint u_TransformIndex;

// ===== VERTEX SHADER OUT =====

out VS_OUT {
    vec3 v_FragPos;
    vec3 v_Normal;
    vec2 v_TexCoords;
    vec4 v_FragPosDirLightSpace;
    vec4 v_FragPosSpotLightSpace;
} vs_out;

// Shadow Mapping
uniform mat4 u_DirLightSpaceMat;
uniform mat4 u_SpotLightSpaceMat;

void main() 
{
    mat4 modelMat  = u_Instances[u_TransformIndex].model;
    mat3 normalMat = mat3(u_Instances[u_TransformIndex].normal);

	vs_out.v_FragPos   	 	   = vec3(modelMat * vec4(a_Pos, 1));
    vs_out.v_Normal    		   = normalMat * a_Normal;
	vs_out.v_TexCoords 		   = a_TexCoords;

	// fragment position in light space
	vs_out.v_FragPosDirLightSpace  = u_DirLightSpaceMat * vec4(vs_out.v_FragPos, 1);
	vs_out.v_FragPosSpotLightSpace = u_SpotLightSpaceMat * vec4(vs_out.v_FragPos, 1);
    
	gl_Position = u_ViewProj * modelMat * vec4(a_Pos, 1.0);		// world space transformation
}


#TYPE FRAGMENT
#version 460 core

// ===== Light Structs =====
struct PointLightData
{
    vec4 positionAndRange; 
    vec4 colorAndPower;    
};
struct DirLightData
{
    vec4 directionAndPower;
    vec4 color;
};
struct SpotLightData
{
    vec4 positionAndLength;
    vec4 directionAndInner;
    vec4 colorAndOuter;
    vec4 powerAndPadding;
};

layout(std140, binding = 0) uniform CameraBuffer
{
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProj;
    vec4 u_CameraPosAndTime;
};
layout(std430, binding = 2) readonly buffer LightBuffer
{
    uint u_DirCount;
    uint u_PointCount;
    uint u_SpotCount;
    uint u_Padding;
    
    DirLightData   u_DirLights[16];
    PointLightData u_PointLights[1024];
    SpotLightData  u_SpotLights[512];
};


in VS_OUT {
    vec3 v_FragPos;
    vec3 v_Normal;
    vec2 v_TexCoords;
    vec4 v_FragPosDirLightSpace;
    vec4 v_FragPosSpotLightSpace;
} fs_in;

out vec4 FragColor;

// Shadow Mapping
uniform sampler2D u_DirDepthMapTex;	    // 10
uniform sampler2D u_SpotDepthMapTex;    // 11
uniform samplerCube u_PointDepthMapTex; // 12
uniform float u_PointFarPlane;

uniform sampler2D u_albedoMap;     uniform bool u_hasAlbedoMap;     uniform vec3  u_albedoFactor;
uniform sampler2D u_normalMap;	   uniform bool u_hasNormalMap;
uniform sampler2D u_metallicMap;   uniform bool u_hasMetallicMap;   uniform float u_metallicFactor;
uniform sampler2D u_roughnessMap;  uniform bool u_hasRoughnessMap;  uniform float u_roughnessFactor;
uniform sampler2D u_aoMap;		   uniform bool u_hasAOMap;		    uniform float u_AOFactor;

float calcShadow(sampler2D shadowMap, vec4 fragPosLightSpace, float bias = 0.0f)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	
	if (projCoords.z > 1.0)
        return 0.0;

	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

float calcPointShadow(vec3 fragPos, vec3 lightPos, float farPlane, float bias = 0.05)
{
    vec3 fragToLight = fragPos - lightPos;

    float closestDepth = texture(u_PointDepthMapTex, fragToLight).r;

    closestDepth *= farPlane;
    
    float currentDepth = length(fragToLight);

    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

vec3 calcDirLight(DirLightData light, vec3 normal, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(-light.directionAndPower.xyz); 
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);

    vec3 diffuse  = light.color.rgb * diff * light.directionAndPower.w;
    vec3 specular = light.color.rgb * spec * light.directionAndPower.w;

    return (diffuse + specular) * (1.0 - shadow);
}

vec3 calcSpotLight(SpotLightData light, vec3 normal, vec3 viewDir, vec3 fragPos, float shadow)
{
    vec3 lightDir = normalize(light.positionAndLength.xyz - fragPos);
    
    // Diffuse & Specular
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    
    // Attenuation (distance)
    float distance = length(light.positionAndLength.xyz - fragPos);
    float attenuation = 1.0 / (distance * distance);
    
    // Spotlight Cone intensity
    float theta = dot(lightDir, normalize(-light.directionAndInner.xyz)); 
    float epsilon = light.directionAndInner.w - light.colorAndOuter.w; // innerCos - outerCos
    float intensity = clamp((theta - light.colorAndOuter.w) / epsilon, 0.0, 1.0); 
    
    vec3 diffuse  = light.colorAndOuter.rgb * diff * light.powerAndPadding.x;
    vec3 specular = light.colorAndOuter.rgb * spec * light.powerAndPadding.x;
    
    return (diffuse + specular) * attenuation * intensity * (1.0 - shadow);
}

vec3 calcPointLight(PointLightData light, vec3 normal, vec3 viewDir, vec3 fragPos, float shadow)
{
    vec3 lightDir = normalize(light.positionAndRange.xyz - fragPos);
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    
    float distance = length(light.positionAndRange.xyz - fragPos);
    float attenuation = 1.0 / (distance * distance);

    float range = light.positionAndRange.w;
    float window = clamp(1.0 - pow(distance / range, 4.0), 0.0, 1.0);
    attenuation *= window * window;
    
    vec3 diffuse  = light.colorAndPower.rgb * diff * light.colorAndPower.w;
    vec3 specular = light.colorAndPower.rgb * spec * light.colorAndPower.w;
    
    return (diffuse + specular) * attenuation * (1.0 - shadow);
}


void main() 
{
	vec3 albedo = u_hasAlbedoMap ? texture(u_albedoMap, fs_in.v_TexCoords).rgb : u_albedoFactor;
	vec3 normal = normalize(fs_in.v_Normal);
	vec3 viewDir = normalize(u_CameraPosAndTime.xyz - fs_in.v_FragPos);

	vec3 totalIllumination = albedo * 0.01; // Base ambient

	const float shadowBias = 0.005;

    // Directional Lights
    for(uint i = 0; i < u_DirCount; i++)
    {
        float shadow = (i == 0)
					   ? calcShadow(u_DirDepthMapTex, fs_in.v_FragPosDirLightSpace, shadowBias) : 0.0;
        totalIllumination += calcDirLight(u_DirLights[i], normal, viewDir, shadow) * albedo;
    }

    // Spot Ligts
	for(uint i = 0; i < u_SpotCount; i++)
    {
        float shadow = (i == 0)
					   ? calcShadow(u_SpotDepthMapTex, fs_in.v_FragPosSpotLightSpace, shadowBias) : 0.0;
        totalIllumination += calcSpotLight(u_SpotLights[i], normal, viewDir, fs_in.v_FragPos, shadow) * albedo;
    }
	
	// Point Lights
	for(uint i = 0; i < u_PointCount; i++)
    {
        float shadow = (i == 0)
					   ? calcPointShadow(fs_in.v_FragPos, u_PointLights[i].positionAndRange.xyz, u_PointFarPlane, shadowBias) : 0.0;
        totalIllumination += calcPointLight(u_PointLights[i], normal, viewDir, fs_in.v_FragPos, shadow) * albedo;
    }

	FragColor = vec4(totalIllumination, 1.0);
}