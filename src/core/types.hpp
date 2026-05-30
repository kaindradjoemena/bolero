// core/types.hpp

#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <numbers>

#include <string>
#include <cstdint>

#include <memory>


namespace blr::core
{


/* ===== SMART POINTERS ===== */
template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;

/* ===== MATH ===== */
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using quat = glm::quat;

inline
float Clamp(float x, float min, float max) { return glm::clamp(x, min, max); }

inline
float DegToRad(float deg) { return glm::radians(deg); }

inline constexpr
float DegToRadConst(float deg) { return glm::radians(deg); }

inline
vec3 QuatToEul(quat q) { return glm::eulerAngles(q); }

inline
quat EulToQuat(vec3 e) { return quat(e); }

inline
vec3 EulToDir(vec3 e) { return glm::normalize(glm::quat(glm::radians(e)) * glm::vec3(0.0f, 0.0f, -1.0f)); }

inline
vec3 Norm(vec3 v) { return glm::normalize(v); }

inline
vec3 Cross(vec3 a, vec3 b) { return glm::cross(a, b); }

inline
mat3 Transpose(const mat3& m) { return glm::transpose(m); } 

inline
mat4 Transpose(const mat4& m) { return glm::transpose(m); }

inline
mat3 Inverse(const mat3& m) { return glm::inverse(m); }

inline
mat4 Inverse(const mat4& m) { return glm::inverse(m); }

inline
mat4 LookAt(const vec3& eye, const vec3& center, const vec3& up) { return glm::lookAt(eye, center, up); }

inline
mat4 Perspective(float rad, float aspect, float near, float far) { return glm::perspective(rad, aspect, near, far); }

inline
mat4 Ortho(float left, float right, float bottom, float top, float near, float far) { return glm::ortho(left, right, bottom, top, near, far); }

/* ===== TRANSFORMS ===== */
struct Transform
{
public:
    vec3 GetPos() const { return m_pos; }
    vec3 GetScl() const { return m_scl; }
    quat GetRot() const { return m_rot; }
    mat4 GetModelMat() const
    {
        if (!m_isDirty)
        {
            m_isDirty = false;
            return m_modelMat;
        }

        mat4 modelMat = mat4(1.0f);
        modelMat      = glm::translate(modelMat, m_pos);
        modelMat     *= glm::mat4_cast(m_rot);
        modelMat      = glm::scale(modelMat, m_scl);
        
        m_isDirty = false;
        m_modelMat = modelMat;

        return m_modelMat;
    }

    void SetPos(vec3 pos) { m_pos = pos; m_isDirty = true; }
    void SetScl(vec3 scl) { m_scl = scl; m_isDirty = true; }
    void SetRot(quat rot) { m_rot = rot; m_isDirty = true; }

private:
    mutable bool m_isDirty = false;
    
    vec3 m_pos = vec3(0.0f);
    vec3 m_scl = vec3(1.0f);
    quat m_rot = quat(1.0f, 0.0f, 0.0f, 0.0f);
    mutable mat4 m_modelMat = mat4(1.0f);
};

/* ===== RENDERER ===== */
// ----- Textures -----
// enum TexSlot
// {
//     SLOT_0 = GL_TEXTURE0,
//     SLOT_1 = GL_TEXTURE1,
//     SLOT_3 = GL_TEXTURE2
// };

enum class ImgFmt
{
    None = 0,
    R8,
    RG8,
    RGB8,
    RGBA8,
    SRGB8,
    SRGBA8,
    RGBA16F,
    RGB16F,
    Depth32F,
    Depth24Stencil8
};

enum class TexFilter
{
    None = 0,
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};

enum class TexWrap
{
    None = 0,
    Repeat,
    ClampToEdge,
    ClampToBorder
};

inline
GLenum ImgFmtToGLFmt(ImgFmt imgFmt)
{
    switch(imgFmt)
    {
        case ImgFmt::R8:              return GL_R8;
        case ImgFmt::RG8:             return GL_RG8;
        case ImgFmt::RGB8:            return GL_RGB8;
        case ImgFmt::RGBA8:           return GL_RGBA8;
        case ImgFmt::RGBA16F:         return GL_RGBA16F;
        case ImgFmt::Depth32F:        return GL_DEPTH_COMPONENT32F;
        case ImgFmt::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
        default:                      return GL_RGB8;
    }
}

inline
GLenum GetGLDataFmt(ImgFmt imgFmt)
{
    switch(imgFmt)
    {
        case ImgFmt::R8:              return GL_RED;
        case ImgFmt::RGB8:            return GL_RGB;
        case ImgFmt::RGBA8:
        case ImgFmt::RGBA16F:         return GL_RGBA;
        case ImgFmt::Depth32F:        return GL_DEPTH_COMPONENT;
        case ImgFmt::Depth24Stencil8: return GL_DEPTH_STENCIL;
        default:                      return GL_RGBA;
    }
}

inline
GLenum TexFilterToGLEnum(TexFilter filter)
{
    switch (filter)
    {
        case TexFilter::Nearest:              return GL_NEAREST;
        case TexFilter::Linear:               return GL_LINEAR;
        case TexFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case TexFilter::LinearMipmapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
        case TexFilter::NearestMipmapLinear:  return GL_NEAREST_MIPMAP_LINEAR;
        case TexFilter::LinearMipmapLinear:   return GL_LINEAR_MIPMAP_LINEAR;
        default:                              return GL_LINEAR;
    }
}

inline
GLenum TextWrapToGLEnum(TexWrap wrap)
{
    switch (wrap)
    {
        case TexWrap::Repeat:        return GL_REPEAT;
        case TexWrap::ClampToEdge:   return GL_CLAMP_TO_EDGE;
        case TexWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
        default:                     return GL_REPEAT;
    }
}

struct TexSpec
{
    uint32_t w          = 1;
    uint32_t h          = 1;
    bool generateMips   = true;
    ImgFmt format       = ImgFmt::RGBA8;
    TexWrap wrapS       = TexWrap::Repeat;
    TexWrap wrapT       = TexWrap::Repeat;
    TexFilter minFilter = TexFilter::LinearMipmapLinear;
    TexFilter magFilter = TexFilter::Linear;
};

struct FBAttachmentSpec
{
    ImgFmt format = ImgFmt::None;
    FBAttachmentSpec() = default;
    FBAttachmentSpec(ImgFmt fmt) : format(fmt) {} 
};

struct FBSpec
{
    uint32_t w = 0;
    uint32_t h = 0;
    std::vector<FBAttachmentSpec> attachments;
};



// ----- Shader Stage -----
enum class ShaderStage
{
    None = 0,
    Vertex,
    Fragment, Pixel,
    Geometry,
    Compute
};

inline
GLenum ShaderStageToGLEnum(ShaderStage stage)
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

inline
ShaderStage ShaderStageFromStr(const std::string& type)
{
    if (type == "VERTEX")   return ShaderStage::Vertex;
    if (type == "FRAGMENT" || 
        type == "PIXEL")    return ShaderStage::Fragment;
    if (type == "GEOMETRY") return ShaderStage::Geometry;
    if (type == "COMPUTE")  return ShaderStage::Compute;

    return ShaderStage::None;
}

// ----- Shader Data Types -----
enum class ShaderDataType
{
    None = 0,
    Float, Float2, Float3, Float4,
    Mat3, Mat4,
    Int, Int2, Int3, Int4,
    Bool
};

inline
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

inline
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

inline
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


} /* namespace blr::core */