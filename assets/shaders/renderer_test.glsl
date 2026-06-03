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

out vec3 v_Normal;

void main() 
{
    mat4 modelMat  = u_Instances[u_TransformIndex].model;
    mat3 normalMat = mat3(u_Instances[u_TransformIndex].normal);

    v_Normal = normalMat * a_Normal;
    
	gl_Position = u_Projection * u_View * modelMat * vec4(a_Pos, 1.0);
}


#TYPE FRAGMENT
#version 460 core

in vec3 v_Normal;

out vec4 FragColor;

void main() 
{
    vec3 norm = normalize(v_Normal);
    vec3 color = norm * 0.5 + 0.5; 
    
    FragColor = vec4(color, 1.0);
}