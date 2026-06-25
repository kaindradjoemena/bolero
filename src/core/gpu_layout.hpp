// core/gpu_layout.hpp

#pragma once

#include "utils/base.hpp"
#include "utils/math.hpp"


namespace blr::core
{


constexpr int MAX_DIR_LIGHTS   = 4;
constexpr int MAX_SPOT_LIGHTS  = 4;
constexpr int MAX_POINT_LIGHTS = 4;

// ===== GPU STRUCT LAYOUT ===============================================================================
struct DirLightData
{
    vec4 directionAndPower; // xyz = direction, w = power
    vec4 colorAndShadow;    // xyz = color,     w = shadow (-1 = no shadows)
};
struct SpotLightData
{
    vec4 positionAndLength; // xyz = position,  w = length
    vec4 directionAndInner; // xyz = direction, w = innerCos
    vec4 colorAndOuter;     // xyz = color,     w = outerCos
    vec4 PowerAndShadow;    // x = power,       y = shadow (-1.0f = no shadows), zw = padding (0.0f)
};
struct PointLightData
{
    vec4 positionAndRange; // xyz = position,               w = range
    vec4 colorAndPower;    // xyz = color,                  w = power
    vec4 shadow;           // x = shadow (-1 = no shadows), yzw = padding (0.0f)
};
struct GPULightBuffer
{
    uint32_t dirCount{0};
    uint32_t pointCount{0};
    uint32_t spotCount{0};
    uint32_t padding{0};
    
    DirLightData   dirLights[MAX_DIR_LIGHTS];
    PointLightData pointLights[MAX_POINT_LIGHTS];
    SpotLightData  spotLights[MAX_SPOT_LIGHTS];
};
struct InstanceData
{
    mat4 model;
    mat4 normal;
};
struct CameraFrameData
{
    mat4 view;
    mat4 projection;
    vec4 cameraPosAndTime; // xyz = position, w = time
};


// ===== GPU DATA LAYOUT ===============================================================================
namespace LayoutLoc
{
    /* VERTEX BUFFER */
    constexpr int POS                = 0;
    constexpr int NORM               = 1;
    constexpr int TEX_COORDS         = 2;
    constexpr int TANGENT            = 3;
    constexpr int BITANGENT          = 4;

    /* MATERIAL MAPS */
    constexpr int ALBEDO_MAP         = 0;
    constexpr int NORMAL_MAP         = 1;
    constexpr int METALLIC_MAP       = 2;
    constexpr int ROUGHNESS_MAP      = 3;
    constexpr int AO_MAP             = 4;

    /* G-BUFFER */
    constexpr int G_ALBEDO_ROUGH     = 0;
    constexpr int G_NORMAL_METAL     = 1;
    constexpr int G_DEPTH            = 2;
    
    /* IBL */
    constexpr int IRRADIANCE_MAP     = 5;
    constexpr int PREFILTER_MAP      = 6;
    constexpr int BRDF_LUT           = 7;
    constexpr int ENV_CUBE_MAP       = 8;

    /* SHADOW MAPS */
    constexpr int DIR_SHADOW_MAPS    = 10;
    constexpr int SPOT_SHADOW_MAPS   = 14;
    constexpr int POINT_SHADOW_MAPS  = 18;
    
    /* SHADOW MASKS */
    constexpr int DIR_SHADOW_MASKS   = 10;
    constexpr int SPOT_SHADOW_MASKS  = 14;
    constexpr int POINT_SHADOW_MASKS = 18;
};

namespace BufferLoc
{
    /* UBO */
    constexpr int CAM_UBO       = 0;

    /* SSBO */
    constexpr int INSTANCE_SSBO = 1;
    constexpr int LIGHT_SSBO    = 2;
};

} /* namespace:: blr::core::LayoutLoc */
