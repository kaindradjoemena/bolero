// core/lights.hpp

#pragma once

#include "types.hpp"


namespace BLR
{


struct LightBase
{
    glm::vec3 color = glm::vec3(1.0f);
    float     power = 10.0f;
};

struct DirLight
{
    LightBase base;
    glm::vec3 direction;
};

struct PointLight
{
    LightBase base;
    glm::vec3 position;
    float     range;
};

struct SpotLight
{
    LightBase base;
    glm::vec3 position;
    glm::vec3 direction;
    float     length;
    float     innerCos;
    float     outerCos;
};


} /* namespace BLR */