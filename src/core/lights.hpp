// core/lights.hpp

#pragma once

#include "utils/math.hpp"

#include <array>


namespace blr::core
{


namespace LightBaseDefaults
{
    static constexpr vec3  COLOR = vec3(1.0f);
    static constexpr float POWER = 10.0f;
};

struct LightBase
{
    vec3 color = LightBaseDefaults::COLOR;
    float power = LightBaseDefaults::POWER;
};

struct DirLight
{
    LightBase base;
    vec3 direction;

    bool castsShadow;
    float shadowSize;
    float shadowNear;
    float shadowFar;

    DirLight()
    : direction(-1.0f, -0.3f, -0.3f)
    , castsShadow(true)
    , shadowSize(20.0f)
    , shadowNear(0.1f)
    , shadowFar(20.0f)
    {
    }

    mat4 GetLightSpaceMat(const vec3& targetPos) const
    {
        vec3 lightDir = Norm(direction);
        vec3 lightPos = targetPos - (lightDir * (shadowFar * 0.5f)); 

        mat4 lightView = LookAt(lightPos, targetPos, vec3(0.0f, 1.0f, 0.0f));
        mat4 lightProj = Ortho(-shadowSize, shadowSize, -shadowSize, shadowSize, shadowNear, shadowFar);
    
        return lightProj * lightView;   // NOTE: could cache this
    }
};

struct PointLight
{
    LightBase base;
    vec3 position;
    float range;

    bool castsShadow;
    float shadowNear;

    PointLight()
    : position(0.0f, 0.0f, 0.0f)
    , range(10.0f)
    , castsShadow(true)
    , shadowNear(0.1f)
    {
    }

    std::array<mat4, 6> GetLightSpaceMatrices() const
    {
        mat4 shadowProj = Perspective(DegToRad(90.0f), 1.0f, shadowNear, range);

        return {
            shadowProj * LookAt(position, position + vec3( 1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),  // +X
            shadowProj * LookAt(position, position + vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)),  // -X
            shadowProj * LookAt(position, position + vec3( 0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)),  // +Y
            shadowProj * LookAt(position, position + vec3( 0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)),  // -Y
            shadowProj * LookAt(position, position + vec3( 0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)),  // +Z
            shadowProj * LookAt(position, position + vec3( 0.0f,  0.0f, -1.0f), vec3(0.0f, -1.0f,  0.0f))}; // -Z
    }
};

struct SpotLight
{
    LightBase base;
    vec3 position;
    vec3 direction;
    float length;
    float innerCos;
    float outerCos;

    bool castsShadow;
    float shadowNear;

    SpotLight()
    : position(0.0f, 0.0f, 0.0f)
    , direction(0.0f, -1.0f, 0.0f)
    , length(10.0f)
    , innerCos(std::cos(DegToRad(12.5f)))
    , outerCos(std::cos(DegToRad(17.5f)))
    , castsShadow(true)
    , shadowNear(0.1f)
    {
    }

    mat4 GetLightSpaceMat() const
    {
        vec3 lightDir = Norm(direction);

        vec3 up = (abs(lightDir.y) > 0.999f) ? vec3(0.0f, 0.0f, 1.0f) : vec3(0.0f, 1.0f, 0.0f);

        mat4 lightView = LookAt(position, position + lightDir, up);
        float fov = acos(outerCos) * 2.0f;
        mat4 lightProj = Perspective(fov, 1.0f, shadowNear, length);

        return lightProj * lightView;
    }
};


} /* namespace blr::core */