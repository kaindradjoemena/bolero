#TYPE VERTEX
#version 460 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_col;

uniform mat4 u_mvp;

out vec3 v_color;

void main()
{
    gl_Position = u_mvp * vec4(a_pos, 1.0);
    v_color = a_col;
}


#TYPE FRAGMENT
#version 460 core
in vec3 v_color;
out vec4 frag_color;

void main()
{
    frag_color = vec4(v_color, 1.0);
}