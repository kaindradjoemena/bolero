// skybox.glsl

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

layout(std140, binding = 0) uniform CameraBuffer
{
    mat4 u_View;
	mat4 u_Projection;
	vec4 u_CameraPosAndTime;
};

out vec3 v_TexCoords;

void main()
{
    v_TexCoords = cubeVertices[gl_VertexID];
    
    mat4 rotView = mat4(mat3(u_View)); 
    vec4 clipPos = u_Projection * rotView * vec4(v_TexCoords, 1.0);
    
    gl_Position = clipPos.xyww; 
}

#TYPE FRAGMENT
#version 460 core

out vec4 FragColor;
in vec3 v_TexCoords;

layout(binding = 6) uniform samplerCube u_PrefilterMap;

uniform float u_EnvironmentBlur  = 0.9;
uniform float u_EnvironmentRot   = 0.0;
uniform float u_EnvironmentPower = 1.0;

mat3 getRotationY(float angle)
{
	float s = sin(angle);
	float c = cos(angle);
	return mat3(c,   0.0, s,
				0.0, 1.0, 0.0,
				-s,  0.0, c);
}

void main()
{
	vec3 rotatedUV = getRotationY(u_EnvironmentRot) * v_TexCoords;
    vec3 envColor = textureLod(u_PrefilterMap, rotatedUV, u_EnvironmentBlur).rgb * u_EnvironmentPower;
	
	FragColor = vec4(envColor, 1.0);
}