// post_process.glsl

#TYPE VERTEX
#version 460 core

out vec2 v_TexCoords;

void main()
{
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    
	v_TexCoords.x = (x + 1.0) * 0.5;
    v_TexCoords.y = (y + 1.0) * 0.5;
    
	gl_Position = vec4(x, y, 0.0, 1.0);
}

#TYPE FRAGMENT
#version 460 core

out vec4 FragColor;

in vec2 v_TexCoords;

uniform sampler2D u_ScreenTexture;
uniform float u_Exposure;

vec3 ACESFilm(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main()
{
    vec3 hdrColor = texture(u_ScreenTexture, v_TexCoords).rgb;

	hdrColor *= u_Exposure;
	
	vec3 mapped = ACESFilm(hdrColor);

    mapped = pow(mapped, vec3(1.0 / 2.2));

    FragColor = vec4(mapped, 1.0);
}