#TYPE VERTEX
#version 460 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projMat;
uniform mat3 u_normMat;

out vec3 v_norm;
out vec3 v_fragPos;

void main()
{
    v_fragPos = vec3(u_modelMat * vec4(a_pos, 1.0f));   // world-space frag pos
    v_norm = u_normMat * a_norm;

    gl_Position = u_projMat * u_viewMat * u_modelMat * vec4(a_pos, 1.0);
}


#TYPE FRAGMENT
#version 460 core
in vec3 v_norm;
in vec3 v_fragPos;

uniform vec3  u_lightCol;
uniform float u_lightPow;
uniform vec3  u_lightPos;
uniform float u_lightRange;
uniform vec3  u_camPos;

out vec4 frag_color;

const vec3 ambientColor = vec3(0.02f, 0.0f, 0.0f);
const vec3 diffuseColor = vec3(0.8f, 0.0f, 0.6f);
const vec3 specColor    = vec3(1.0f);
const float shininess   = 16.0f;
const float screenGamma = 2.2f;

void main()
{
    vec3 normal = normalize(v_norm);
    vec3 lightVec = u_lightPos - v_fragPos;
    float distanceSq = dot(lightVec, lightVec);
    vec3 lightDir = normalize(lightVec);

    float attenuation = u_lightPow / max(distanceSq, 0.0001f);

    float rangeSq = u_lightRange * u_lightRange;
    float distSqOverRangeSq = distanceSq / rangeSq;
    float windowing = clamp(1.0f - (distSqOverRangeSq * distSqOverRangeSq), 0.0f, 1.0f);
    windowing = windowing * windowing; 
    attenuation *= windowing;

    // Diffuse
    float lambertian = max(dot(lightDir, normal), 0.0f);

    // Specular
    vec3 viewDir = normalize(u_camPos - v_fragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(halfDir, normal), 0.0f);

    float specular = pow(specAngle, shininess);
    
    float specMask = step(0.001f, lambertian); 
    specular *= specMask;

    vec3 colorLinear = ambientColor +
                       (diffuseColor * lambertian * u_lightCol * attenuation) +
                       (specColor * specular * u_lightCol * attenuation);
                    
    // Gamma correction
    frag_color = vec4(pow(colorLinear, vec3(1.0f / screenGamma)), 1.0f);
}