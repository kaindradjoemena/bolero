// core/shader.hpp

#pragma once

#include <glad/glad.h>

#include "types.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <filesystem>

namespace BLR
{


class Shader
{
public:
    static constexpr const char* TYPE_TOKEN = "#TYPE";

public:
    Shader(const std::filesystem::path& filePath);
    ~Shader();

    // Prevent copying
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Allow moving
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void Bind() const;
    void Unbind() const;

    void SetInt(std::string_view name, int value);
    void SetFloat(std::string_view name, float value);
    void SetVec3(std::string_view name, const vec3& value);
    void SetVec4(std::string_view name, const vec4& value);
    void SetMat3(std::string_view name, const mat3& value);
    void SetMat4(std::string_view name, const mat4& value);

    GLuint GetRendererID() const { return m_rendererID; }

private:
    GLuint m_rendererID{0};
    std::unordered_map<std::string, GLint> m_uniformLocCache;

    std::string ReadFile(const std::filesystem::path& filePath);
    std::unordered_map<GLenum, std::string> PreProcess(const std::string& src);
    void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);

    GLint GetUniformLocation(std::string_view name);
    void CheckCompileErrors(GLuint object, std::string_view type);
};


} /* namespace BLR */