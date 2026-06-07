// core/shader.hpp

#pragma once

#include <glad/glad.h>

#include "utils/base.hpp"
#include "utils/math.hpp"

#include "resource.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <filesystem>


namespace blr::core
{


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


class Shader : public Resource
{
public:
    static constexpr const char* TYPE_TOKEN = "#TYPE";

public:
    ~Shader();

    // Prevent copying
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Allow moving
    Shader(Shader&& other) = default;
    Shader& operator=(Shader&& other) = default;

    void Bind() const;
    void Unbind() const;

    void SetBool(std::string_view name, bool value);
    void SetInt(std::string_view name, int value);
    void SetUInt(std::string_view name, unsigned int value);
    void SetFloat(std::string_view name, float value);
    void SetVec3(std::string_view name, const vec3& value);
    void SetVec4(std::string_view name, const vec4& value);
    void SetMat3(std::string_view name, const mat3& value);
    void SetMat4(std::string_view name, const mat4& value);

    bool Reload();

    GLuint GetRendererID() const { return m_rendererID; }

private:
    std::filesystem::path m_filePath;

    GLuint m_rendererID{0};
    std::unordered_map<std::string, GLint> m_uniformLocCache;

    std::string ReadFile(const std::filesystem::path& filePath);
    std::unordered_map<GLenum, std::string> PreProcess(const std::string& src);
    GLuint Compile(const std::unordered_map<GLenum, std::string>& shaderSources);

    GLint GetUniformLocation(std::string_view name);
    void CheckCompileErrors(GLuint object, std::string_view type);

// Construction must be called by the AssetManager class through the Create method
friend class AssetManager;
protected:
    Shader(const std::filesystem::path& filePath);
    static Ref<Shader> Create(const std::filesystem::path& filePath)
    {
        return std::shared_ptr<Shader>(new Shader(filePath));
    }
};


} /* namespace blr::core */