// lighting_pass.glsl

#TYPE VERTEX
#version 460 core

out vec2 TexCoords;

void main() 
{
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    
    TexCoords = vec2((x + 1.0) * 0.5, (y + 1.0) * 0.5);
    gl_Position = vec4(x, y, 0.0, 1.0);
}


#TYPE FRAGMENT
#version 460 core

layout(location = 0) out vec4 u_DirShadowMask;
layout(location = 1) out vec4 u_SpotShadowMask;
layout(location = 2) out vec4 u_PointShadowMask;

const float PI = 3.141592653589793;

// ===== Light Structs =====
struct DirLightData
{
    vec4 directionAndPower; // xyz = direction, w = power
    vec4 colorAndShadow;    // xyz = color,     w = shadow (-1 = no shadows)
};
struct PointLightData		
{
    vec4 positionAndRange; 	// xyz = position,               w = range
    vec4 colorAndPower;    	// xyz = color,                  w = power
    vec4 shadow;           	// x = shadow (-1 = no shadows), yzw = padding (0.0f)
};
struct SpotLightData		
{
    vec4 positionAndLength; // xyz = position,  w = length
    vec4 directionAndInner; // xyz = direction, w = innerCos
    vec4 colorAndOuter;     // xyz = color,     w = outerCos
    vec4 powerAndShadow;    // x = power,       y = shadow (-1.0f = no shadows), zw = padding (0.0f)
};


layout(std140, binding = 0) uniform CameraBuffer
{
    mat4 u_View;
    mat4 u_Projection;
    vec4 u_CameraPosAndTime;
};
layout(std430, binding = 2) readonly buffer LightBuffer
{
    uint u_DirCount;
    uint u_PointCount;
    uint u_SpotCount;
    uint u_Padding;
    
    DirLightData   u_DirLights[4];
    PointLightData u_PointLights[4];
    SpotLightData  u_SpotLights[4];
};

in vec2 TexCoords;

// G-Buffer
layout(binding = 1) uniform sampler2D gNormalMetal;
layout(binding = 2) uniform sampler2D gDepth;

// Shadow Maps
layout(binding = 10) uniform sampler2D   u_DirShadowMaps[4];
layout(binding = 14) uniform sampler2D   u_SpotShadowMaps[4];
layout(binding = 18) uniform samplerCube u_PointShadowMaps[4];

uniform mat4 u_InvViewProj;

uniform mat4 u_DirLightSpaceMat[4];
uniform mat4 u_SpotLightSpaceMat[4];


vec3 ReconstructWorldPos(vec2 uv, float depth);

const float MIN_BIAS = 0.00001;
const float MAX_BIAS = 0.001;
float calcDirShadow(sampler2D shadowMap, vec4 fragPosLightSpace, float bias);
float calcSpotShadow(sampler2D shadowMap, vec4 fragPosLightSpace, float bias);
float calcPointShadow(samplerCube shadowMap, vec3 fragPos, vec3 lightPos, float farPlane, float bias);
float pcf(sampler2D depthMap, vec4 fragPosLightSpace, float bias);
float pcf(samplerCube depthMap, vec3 worldPos, vec3 lightPos, float farPlane, float bias);


void main() 
{
    float depth = texture(gDepth, TexCoords).r;
    if (depth == 1.0)
        discard;

    u_DirShadowMask   = vec4(0.0);
    u_SpotShadowMask  = vec4(0.0);
    u_PointShadowMask = vec4(0.0);

    vec4 normalMetal = texture(gNormalMetal, TexCoords);

    vec3  N         = normalize(normalMetal.rgb);
    float METALLIC  = normalMetal.a;

    // World pos from depth
    vec3 worldPos = ReconstructWorldPos(TexCoords, depth);

    // Directional Lights
	for	(uint i = 0; i < u_DirCount; i++)
	{
		float shadow = 0.0;
		int shadowIdx = int(u_DirLights[i].colorAndShadow.w);

		if (shadowIdx >= 0)
		{
            vec4 fragPosLightSpace = u_DirLightSpaceMat[shadowIdx] * vec4(worldPos, 1.0);
			vec3 lightDir = normalize(-u_DirLights[i].directionAndPower.xyz);
			float dynamicBias = max(MAX_BIAS * (1.0 - dot(N, lightDir)), MIN_BIAS);

			shadow = calcDirShadow(u_DirShadowMaps[shadowIdx], fragPosLightSpace, dynamicBias);

            u_DirShadowMask[shadowIdx] = shadow;
		}
	}

    // Spot Ligts
	for (uint i = 0; i < u_SpotCount; i++)
	{
		float shadow = 0.0;
		int shadowIdx = int(u_SpotLights[i].powerAndShadow.y);

		if (shadowIdx >= 0)
		{
            vec4 fragPosLightSpace = u_SpotLightSpaceMat[shadowIdx] * vec4(worldPos, 1.0);
            vec3 lightDir = normalize(u_SpotLights[i].positionAndLength.xyz - worldPos);
			float dynamicBias = max(MAX_BIAS * (1.0 - dot(N, lightDir)), MIN_BIAS);

			shadow = calcSpotShadow(u_SpotShadowMaps[shadowIdx], fragPosLightSpace, dynamicBias);

            u_SpotShadowMask[shadowIdx] = shadow;
		}
	}

	// Point Lights Loop
	for (uint i = 0; i < u_PointCount; i++)
	{
		float shadow = 0.0;
		int shadowIdx = int(u_PointLights[i].shadow.x);

		if (shadowIdx >= 0)
        {
			vec3 lightDir = normalize(u_PointLights[i].positionAndRange.xyz - worldPos);
			float dynamicBias = max(MAX_BIAS * (1.0 - dot(N, lightDir)), MIN_BIAS);
		
			float lightRange = u_PointLights[i].positionAndRange.w;
			shadow = calcPointShadow(u_PointShadowMaps[shadowIdx], worldPos, u_PointLights[i].positionAndRange.xyz, lightRange, dynamicBias);
		
            u_PointShadowMask[shadowIdx] = shadow;
        }
	}
}


vec3 ReconstructWorldPos(vec2 uv, float depth)
{
    float z = depth * 2.0 - 1.0;
    vec4 clipSpacePos = vec4(uv * 2.0 - 1.0, z, 1.0);

	vec4 worldSpacePos = u_InvViewProj * clipSpacePos;

    return worldSpacePos.xyz / worldSpacePos.w;
}


// ===== SHADOW CALC =====
float calcDirShadow(sampler2D shadowMap, vec4 fragPosLightSpace, float bias)
{
    return pcf(shadowMap, fragPosLightSpace, bias);
}

float calcSpotShadow(sampler2D shadowMap, vec4 fragPosLightSpace, float bias)
{
    return pcf(shadowMap, fragPosLightSpace, bias);
}

float calcPointShadow(samplerCube shadowMap, vec3 fragPos, vec3 lightPos, float farPlane, float bias)
{
    return pcf(shadowMap, fragPos, lightPos, farPlane, bias);
}

// ===== PCF CALC =====
float pcf(sampler2D shadowMap, vec4 fragPosLightSpace, float bias)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	if (projCoords.z > 1.0)
		return 0.0;

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	return shadow / 9.0;
}

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float pcf(samplerCube shadowMap, vec3 fragPos, vec3 lightPos, float farPlane, float bias)
{
	vec3 fragToLight = fragPos - lightPos;
	float currentDepth = length(fragToLight);

	float shadow = 0.0;
	int samples = 20;
	float viewDistance = length(u_CameraPosAndTime.xyz - fragPos);
	float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0;

	for (int i = 0; i < samples; i++)
	{
		float closestDepth = texture(shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= farPlane;
		if(currentDepth - bias > closestDepth)
			shadow += 1.0;
	}
	return shadow / float(samples);
}