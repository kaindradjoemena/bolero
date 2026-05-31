#TYPE VERTEX
#version 460 core
layout (location = 0) in vec3 a_Pos;

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

void main() 
{
	// Transform to world space
    gl_Position = u_Instances[u_TransformIndex].model * vec4(a_Pos, 1.0);
}

#TYPE GEOMETRY
#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 u_ShadowMatrices[6];

out vec4 FragPos;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i)
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = u_ShadowMatrices[face] * FragPos;
            EmitVertex();
        }    

        EndPrimitive();
    }
}

#TYPE FRAGMENT
#version 460 core
in vec4 FragPos;

uniform vec3 u_LightPos;
uniform float u_FarPlane;

void main()
{
    float lightDistance = length(FragPos.xyz - u_LightPos);
    
    lightDistance = lightDistance / u_FarPlane;
    
    gl_FragDepth = lightDistance;
}