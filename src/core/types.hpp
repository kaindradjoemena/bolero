// core/types.hpp

#pragma once

#include <string>
#include <cstdint>

namespace BLR
{


enum class ShaderStage
{
    None = 0, Vertex, Fragment, Pixel, Geometry, Compute
};

static GLenum ShaderStageToGLEnum(ShaderStage stage)
{
    switch (stage)
    {
        case ShaderStage::Vertex:   return GL_VERTEX_SHADER;
        case ShaderStage::Fragment:
        case ShaderStage::Pixel:    return GL_FRAGMENT_SHADER;
        case ShaderStage::Geometry: return GL_GEOMETRY_SHADER;
        case ShaderStage::Compute:  return GL_COMPUTE_SHADER;
        case ShaderStage::None:     break;
    }
    return 0;
}

static ShaderStage ShaderStageFromStr(const std::string& type)
{
    if (type == "VERTEX")   return ShaderStage::Vertex;
    if (type == "FRAGMENT" || 
        type == "PIXEL")    return ShaderStage::Fragment;
    if (type == "GEOMETRY") return ShaderStage::Geometry;
    if (type == "COMPUTE")  return ShaderStage::Compute;

    return ShaderStage::None;
}


enum class ShaderDataType
{
    None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
};

constexpr uint32_t ShaderDataTypeSize(ShaderDataType type)
{
    switch (type)
    {
        case ShaderDataType::Float:    return 4;
        case ShaderDataType::Float2:   return 4 * 2;
        case ShaderDataType::Float3:   return 4 * 3;
        case ShaderDataType::Float4:   return 4 * 4;
        case ShaderDataType::Mat3:     return 4 * 3 * 3;
        case ShaderDataType::Mat4:     return 4 * 4 * 4;
        case ShaderDataType::Int:      return 4;
        case ShaderDataType::Int2:     return 4 * 2;
        case ShaderDataType::Int3:     return 4 * 3;
        case ShaderDataType::Int4:     return 4 * 4;
        case ShaderDataType::Bool:     return 1;
        case ShaderDataType::None:     return 0;
    }
    return 0;
}

constexpr uint32_t ShaderDataTypeComponentCount(ShaderDataType type)
{
    switch (type)
    {
        case ShaderDataType::Float:    return 1;
        case ShaderDataType::Float2:   return 2;
        case ShaderDataType::Float3:   return 3;
        case ShaderDataType::Float4:   return 4;
        case ShaderDataType::Mat3:     return 3; // 3 * float3
        case ShaderDataType::Mat4:     return 4; // 4 * float4
        case ShaderDataType::Int:      return 1;
        case ShaderDataType::Int2:     return 2;
        case ShaderDataType::Int3:     return 3;
        case ShaderDataType::Int4:     return 4;
        case ShaderDataType::Bool:     return 1;
        case ShaderDataType::None:     return 0;
    }
    return 0;
}

constexpr GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
{
    switch (type)
    {
        case ShaderDataType::Float:    return GL_FLOAT;
        case ShaderDataType::Float2:   return GL_FLOAT;
        case ShaderDataType::Float3:   return GL_FLOAT;
        case ShaderDataType::Float4:   return GL_FLOAT;
        case ShaderDataType::Mat3:     return GL_FLOAT_MAT3;
        case ShaderDataType::Mat4:     return GL_FLOAT_MAT4;
        case ShaderDataType::Int:      return GL_INT;
        case ShaderDataType::Int2:     return GL_INT_VEC2;
        case ShaderDataType::Int3:     return GL_INT_VEC3;
        case ShaderDataType::Int4:     return GL_INT_VEC4;
        case ShaderDataType::Bool:     return GL_BOOL;
        case ShaderDataType::None:     return GL_NONE;
    }
    return 0;
}

} /* namespace BLR */