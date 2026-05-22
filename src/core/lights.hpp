// core/lights.hpp

#pragma once

#include "types.hpp"


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
};

struct PointLight
{
    LightBase base;
    vec3 position;
    float range;
};

struct SpotLight
{
    LightBase base;
    vec3 position;
    vec3 direction;
    float length;
    float innerCos;
    float outerCos;
};


} /* namespace blr::core */