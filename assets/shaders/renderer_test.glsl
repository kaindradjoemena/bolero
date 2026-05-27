#TYPE VERTEX
#version 460 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoords;

struct InstanceData
{
    mat4 model;
    mat4 normal;
};

layout(std430, binding = 0) readonly buffer InstanceBuffer
{
    InstanceData instances[];
};

uniform uint u_TransformIndex;
uniform mat4 u_ViewProj;

out vec3 v_Normal;

void main() 
{
    mat4 modelMat  = instances[u_TransformIndex].model;
    mat3 normalMat = mat3(instances[u_TransformIndex].normal);

    v_Normal = normalMat * a_Normal;
    
	gl_Position = u_ViewProj * modelMat * vec4(a_Pos, 1.0f);
}


#TYPE FRAGMENT
#version 460 core

in vec3 v_Normal;

out vec4 FragColor;

void main() 
{
    vec3 norm = normalize(v_Normal);
    vec3 color = norm * 0.5f + 0.5f; 
    
    FragColor = vec4(color, 1.0f);
}