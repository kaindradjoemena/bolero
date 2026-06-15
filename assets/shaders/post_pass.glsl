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
	x *= 0.6;

    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
	
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 hdrColor = texture(u_ScreenTexture, v_TexCoords).rgb;

	hdrColor *= u_Exposure;
	
	vec3 mapped = ACESFilm(hdrColor);

    mapped = pow(mapped, vec3(1.0 / 2.2));

    FragColor = vec4(mapped, 1.0);
}