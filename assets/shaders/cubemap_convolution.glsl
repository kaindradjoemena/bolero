// cubemap convolution

#TYPE VERTEX
#version 460 core

const vec3 cubeVertices[36] = vec3[36](
    vec3(-1.0,  1.0, -1.0), vec3(-1.0, -1.0, -1.0), vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0), vec3( 1.0,  1.0, -1.0), vec3(-1.0,  1.0, -1.0),
    vec3(-1.0, -1.0,  1.0), vec3(-1.0, -1.0, -1.0), vec3(-1.0,  1.0, -1.0),
    vec3(-1.0,  1.0, -1.0), vec3(-1.0,  1.0,  1.0), vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0, -1.0), vec3( 1.0, -1.0,  1.0), vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0), vec3( 1.0,  1.0, -1.0), vec3( 1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0), vec3(-1.0,  1.0,  1.0), vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0), vec3( 1.0, -1.0,  1.0), vec3(-1.0, -1.0,  1.0),
    vec3(-1.0,  1.0, -1.0), vec3( 1.0,  1.0, -1.0), vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0), vec3(-1.0,  1.0,  1.0), vec3(-1.0,  1.0, -1.0),
    vec3(-1.0, -1.0, -1.0), vec3(-1.0, -1.0,  1.0), vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0), vec3(-1.0, -1.0,  1.0), vec3( 1.0, -1.0,  1.0)
);

out vec3 v_WorldPos;

void main()
{
	v_WorldPos = cubeVertices[gl_VertexID];

    gl_Position = vec4(v_WorldPos, 1.0);
}

#TYPE GEOMETRY
#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 u_ViewProjMatrices[6];

in vec3 v_WorldPos[]; 

out vec3 g_WorldPos; 

void main()
{
    for(int face = 0; face < 6; face++)
    {
        gl_Layer = face; 
        for(int i = 0; i < 3; i++)
        {
            g_WorldPos = v_WorldPos[i];
            gl_Position = u_ViewProjMatrices[face] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}

#TYPE FRAGMENT
#version 460 core

in vec3 g_WorldPos;

out vec4 FragColor;

layout(binding = 18) uniform samplerCube u_EnvMap;

const float PI = 3.14159265359;

void main()
{    
	vec3 N = normalize(g_WorldPos);

    vec3 irradiance = vec3(0.0);   

    vec3 up    = vec3(0.0, 1.0, 0.0);			// might need to check up on this
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(u_EnvMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    FragColor = vec4(irradiance, 1.0);
}