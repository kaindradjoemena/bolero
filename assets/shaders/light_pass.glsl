// light_pass.glsl

/*
 * PBR Implementation credit: https://learnopengl.com/code_viewer_gh.php?code=src/6.pbr/1.2.lighting_textured/1.2.pbr.fs
 * found in chapter: https://learnopengl.com/PBR/Lighting
*/

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

const float PI = 3.141592653589793;

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


// Shadow Calculation Helpers
float calcShadow(sampler2D shadowMap, vec4 fragPosLightSpace, float bias = 0.05);
float calcPointShadow(vec3 fragPos, vec3 lightPos, float farPlane, float bias = 0.05);

// Cook-Torrance BRDF Helpers
float NDF_GGXTR(vec3 N, vec3 H, float roughness);
float G_SchlickGGX(float NdotV, float roughness);
float G_Smith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 F_Schlick(float cosTheta, vec3 F0);

// Material Reflections
vec3 calcDirLight(DirLightData light, vec3 N, vec3 V, float shadow, vec3 ALBEDO, float ROUGHNESS, float METALLIC, vec3 F0);
vec3 calcSpotLight(SpotLightData light, vec3 N, vec3 V, vec3 fragPos, float shadow, vec3 ALBEDO, float ROUGHNESS, float METALLIC, vec3 F0);
vec3 calcPointLight(PointLightData light, vec3 N, vec3 V, vec3 fragPos, float shadow, vec3 ALBEDO, float ROUGHNESS, float METALLIC, vec3 F0);


void main() 
{
	// note: is the pow(texture, 2.2) necessary?
	vec3  ALBEDO    = u_hasAlbedoMap ? pow(texture(u_albedoMap, fs_in.v_TexCoords).rgb, vec3(2.2)) : u_albedoFactor;
    vec3  NORMAL    = normalize(fs_in.v_Normal);
    float METALLIC  = u_hasMetallicMap ? texture(u_metallicMap, fs_in.v_TexCoords).b : u_metallicFactor;		// Blue channel
    float ROUGHNESS = u_hasRoughnessMap ? texture(u_roughnessMap, fs_in.v_TexCoords).g : u_roughnessFactor;		// Green channel
    float AO        = u_hasAOMap ? texture(u_aoMap, fs_in.v_TexCoords).r : u_AOFactor;							// Red channel

	ROUGHNESS = clamp(ROUGHNESS, 0.03, 1.0);  // prevent infinitely small specular points

	vec3 N = normalize(fs_in.v_Normal);
	vec3 V = normalize(u_CameraPosAndTime.xyz - fs_in.v_FragPos);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, ALBEDO, METALLIC);
	
	vec3 Lo = vec3(0.0);

	const float shadowBias = 0.005;

	// ----- PBR Reflections -----
    // Directional Lights
    for(uint i = 0; i < u_DirCount; i++)
    {
        float shadow = (i == 0)
					   ? calcShadow(u_DirDepthMapTex, fs_in.v_FragPosDirLightSpace, shadowBias) : 0.0;
        Lo += calcDirLight(u_DirLights[i], N, V, shadow, ALBEDO, ROUGHNESS, METALLIC, F0);
    }
    // Spot Ligts
	for(uint i = 0; i < u_SpotCount; i++)
    {
        float shadow = (i == 0)
					   ? calcShadow(u_SpotDepthMapTex, fs_in.v_FragPosSpotLightSpace, shadowBias) : 0.0;
        Lo += calcSpotLight(u_SpotLights[i], N, V, fs_in.v_FragPos, shadow, ALBEDO, ROUGHNESS, METALLIC, F0);
    }
	// Point Lights
	for(uint i = 0; i < u_PointCount; i++)
    {
        float shadow = (i == 0)
					   ? calcPointShadow(fs_in.v_FragPos, u_PointLights[i].positionAndRange.xyz, u_PointFarPlane, shadowBias) : 0.0;
        Lo += calcPointLight(u_PointLights[i], N, V, fs_in.v_FragPos, shadow, ALBEDO, ROUGHNESS, METALLIC, F0);
    }

	// No IBL, use predefined ambient
	vec3 ambient = vec3(0.03) * ALBEDO * AO;
	vec3 color = ambient + Lo;

	FragColor = vec4(color, 1.0);
}



// ===== PBR MATERIAL REFLECTION HELPERS =====
vec3 calcDirLight(DirLightData light, vec3 N, vec3 V, float shadow, vec3 ALBEDO, float ROUGHNESS, float METALLIC, vec3 F0)
{
    vec3 L = normalize(-light.directionAndPower.xyz);
    vec3 H = normalize(V + L);

    vec3 radiance = light.color.rgb * light.directionAndPower.w;
    
    // Cook-Torrance BRDF
    float NDF = NDF_GGXTR(N, H, ROUGHNESS);   
    float G   = G_Smith(N, V, L, ROUGHNESS);      
    vec3  F   = F_Schlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - METALLIC;	  
    
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; 
    vec3 specular     = numerator / denominator;
    
    float NdotL = max(dot(N, L), 0.0);
    
    return (kD * ALBEDO / PI + specular) * radiance * NdotL * (1.0 - shadow);
}

vec3 calcSpotLight(SpotLightData light, vec3 N, vec3 V, vec3 fragPos, float shadow, vec3 ALBEDO, float ROUGHNESS, float METALLIC, vec3 F0)
{
    vec3 L = normalize(light.positionAndLength.xyz - fragPos);
    vec3 H = normalize(V + L);
    
    // Distance Attenuation
    float dist = length(light.positionAndLength.xyz - fragPos);
    float attenuation = 1.0 / (dist * dist);
    
    // Spotlight Cone Intensity
    float theta = dot(L, normalize(-light.directionAndInner.xyz)); 
    float epsilon = light.directionAndInner.w - light.colorAndOuter.w; // innerCos - outerCos
    float intensity = clamp((theta - light.colorAndOuter.w) / epsilon, 0.0, 1.0); 
    
    vec3 radiance = light.colorAndOuter.rgb * light.powerAndPadding.x * attenuation * intensity;
    
    // Cook-Torrance BRDF
    float NDF = NDF_GGXTR(N, H, ROUGHNESS);   
    float G   = G_Smith(N, V, L, ROUGHNESS);      
    vec3  F   = F_Schlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - METALLIC;	  
    
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; 
    vec3 specular     = numerator / denominator;
    
    float NdotL = max(dot(N, L), 0.0);
    
    return (kD * ALBEDO / PI + specular) * radiance * NdotL * (1.0 - shadow);
}

vec3 calcPointLight(PointLightData light, vec3 N, vec3 V, vec3 fragPos, float shadow, vec3 ALBEDO, float ROUGHNESS, float METALLIC, vec3 F0)
{
    vec3 L = normalize(light.positionAndRange.xyz - fragPos);
    vec3 H = normalize(V + L);
    
    // Distance Attenuation
    float dist = length(light.positionAndRange.xyz - fragPos);
    float attenuation = 1.0 / (dist * dist);
    
    vec3 radiance = light.colorAndPower.rgb * light.colorAndPower.w * attenuation;

    // Cook-Torrance BRDF
    float NDF = NDF_GGXTR(N, H, ROUGHNESS);   
    float G   = G_Smith(N, V, L, ROUGHNESS);      
    vec3  F   = F_Schlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - METALLIC;	  
    
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; 
    vec3 specular     = numerator / denominator;
    
    float NdotL = max(dot(N, L), 0.0);
    
    return (kD * ALBEDO / PI + specular) * radiance * NdotL * (1.0 - shadow);
}

// ===== COOK-TORRANCE BRDF HELPERS =====
float NDF_GGXTR(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float G_SchlickGGX(float NdotV, float roughness)
{
	// For Direct Lighting, k_dir = (a + 1)^2 / 8
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float G_Smith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = G_SchlickGGX(NdotV, roughness);
    float ggx1 = G_SchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 F_Schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ===== SHADOW FACTOR HELPERS =====
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