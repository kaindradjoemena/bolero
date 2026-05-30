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
    vec4 v_FragPosLightSpace;
} vs_out;


uniform mat4 u_LightSpaceMat;	// shadow mapping

void main() 
{
    mat4 modelMat  = u_Instances[u_TransformIndex].model;
    mat3 normalMat = mat3(u_Instances[u_TransformIndex].normal);

	vs_out.v_FragPos   	 	   = vec3(modelMat * vec4(a_Pos, 1));
    vs_out.v_Normal    		   = normalMat * a_Normal;
	vs_out.v_TexCoords 		   = a_TexCoords;
	vs_out.v_FragPosLightSpace = u_LightSpaceMat * vec4(vs_out.v_FragPos, 1);	// fragment position in light space
    
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
    vec4 v_FragPosLightSpace;
} fs_in;

out vec4 FragColor;

uniform sampler2D u_depthMapTex;	// shadow mapping

uniform sampler2D u_albedoMap;     uniform bool u_hasAlbedoMap;     uniform vec3  u_albedoFactor;
uniform sampler2D u_normalMap;	   uniform bool u_hasNormalMap;
uniform sampler2D u_metallicMap;   uniform bool u_hasMetallicMap;   uniform float u_metallicFactor;
uniform sampler2D u_roughnessMap;  uniform bool u_hasRoughnessMap;  uniform float u_roughnessFactor;
uniform sampler2D u_aoMap;		   uniform bool u_hasAOMap;		    uniform float u_AOFactor;

float calcShadow(vec4 fragPosLightSpace)
{
	// fragment position in light space
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	
	// depth from the nearest fragment to the lights position
	float closestDepth = texture(u_depthMapTex, projCoords.xy).r;
	// depth from the currently viewed fragment to the lights position
	float currentDepth = projCoords.z;

	float bias = 0.00001;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	
	if (projCoords.z > 1.0)
        shadow = 0.0;
	
	return shadow;
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

void main() 
{
	vec3 albedo = u_hasAlbedoMap ? texture(u_albedoMap, fs_in.v_TexCoords).rgb : u_albedoFactor;
	vec3 normal = normalize(fs_in.v_Normal);
	vec3 viewDir = normalize(u_CameraPosAndTime.xyz - fs_in.v_FragPos);

	vec3 totalIllumination = albedo * 0.05; 

    for(uint i = 0; i < u_DirCount; i++)
    {
        float shadow = (i == 0) ? calcShadow(fs_in.v_FragPosLightSpace) : 0.0;
        
        totalIllumination += calcDirLight(u_DirLights[i], normal, viewDir, shadow) * albedo;
    }

	FragColor = vec4(totalIllumination, 1.0);
	//FragColor = vec4(1.0, 0.0, 1.0f, 1.0);
}