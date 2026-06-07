// shadow_pass.glsl

#TYPE VERTEX
#version 460 core
layout (location = 0) in vec3 a_Pos;

// ===== Instance Mats =====
struct InstanceData
{
    mat4 model;
    mat4 normal;
};


layout(std430, binding = 1) readonly buffer InstanceBuffer
{
    InstanceData u_Instances[];
};

uniform uint u_TransformIndex;
uniform mat4 u_LightSpaceMat;

void main() 
{
	mat4 modelMat = u_Instances[u_TransformIndex].model;

	gl_Position = u_LightSpaceMat * modelMat * vec4(a_Pos, 1.0);
}


#TYPE FRAGMENT
#version 460 core

out vec4 FragColor;

void main()
{
}