// core/shader.cpp

#include "shader.hpp"
#include "types.hpp"

#include <fstream>
#include <utility>
#include <vector>
#include <iostream>
#include <cstring>

namespace BLR
{


Shader::Shader(const std::filesystem::path& filePath)
{
    std::string src = ReadFile(filePath);
    auto shaderSources = PreProcess(src);
    Compile(shaderSources);
}

Shader::~Shader()
{
    if (m_rendererID != 0)
        glDeleteProgram(m_rendererID);
}

Shader::Shader(Shader&& other) noexcept
    : m_rendererID(std::exchange(other.m_rendererID, 0))
    , m_uniformLocCache(std::move(other.m_uniformLocCache)) {}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other)
    {
        if (m_rendererID != 0)
            glDeleteProgram(m_rendererID);
        
        m_rendererID      = std::exchange(other.m_rendererID, 0);
        m_uniformLocCache = std::move(other.m_uniformLocCache);
    }

    return *this;
}


void Shader::Bind() const
{
    glUseProgram(m_rendererID);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}


std::string Shader::ReadFile(const std::filesystem::path& filePath)
{
    std::string result;
    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    if (in)
    {
        in.seekg(0, std::ios::end);
        size_t size = in.tellg();
        if (size != (size_t)-1)
        {
            result.resize(size);
            in.seekg(0, std::ios::beg);
            in.read(&result[0], size);
        }
        else
        {
            std::cerr << "Could not read from file: " << filePath << std::endl;
        }
    }
    else
    {
        std::cerr << "Could not open file: " << filePath << std::endl;
    }
    return result;
}

std::unordered_map<GLenum, std::string> Shader::PreProcess(const std::string& src)
{
    std::unordered_map<GLenum, std::string> shaderSources;

    size_t tokenLen = strlen(TYPE_TOKEN);
    size_t pos = src.find(TYPE_TOKEN, 0);    // find the first '#TYPE'

    while (pos != std::string::npos)
    {
        size_t eol   = src.find_first_of("\r\n", pos);     // end of line
        size_t begin = pos + tokenLen + 1;
        
        std::string typeStr = src.substr(begin, eol - begin);
        typeStr.erase(typeStr.find_last_not_of(" \n\r\t") + 1);     // NOTE: could be more robust

        size_t nextLinePos = src.find_first_not_of("\r\n", eol);    // start of shader code
        pos = src.find(TYPE_TOKEN, nextLinePos);     // check for another '#TYPE' on the next line

        shaderSources[ShaderStageToGLEnum(ShaderStageFromStr(typeStr))] = (pos == std::string::npos) 
            ? src.substr(nextLinePos) 
            : src.substr(nextLinePos, pos - nextLinePos);
    }

    return shaderSources;
}

void Shader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
{
    GLuint program = glCreateProgram();
    std::vector<GLuint> glShaderIDs;
    glShaderIDs.reserve(shaderSources.size());

    // 1. Compile each shader stage
    for (auto& [type, source] : shaderSources)
    {
        GLuint shader = glCreateShader(type);
        const char* sourceCStr = source.c_str();
        glShaderSource(shader, 1, &sourceCStr, 0);
        glCompileShader(shader);

        CheckCompileErrors(shader, "SHADER_STAGE");

        glAttachShader(program, shader);
        glShaderIDs.push_back(shader);
    }

    // 2. Link the program
    glLinkProgram(program);
    CheckCompileErrors(program, "PROGRAM");

    // 3. Detach and clean up individual shaders to prevent memory leaks
    for (auto id : glShaderIDs)
    {
        glDetachShader(program, id);
        glDeleteShader(id);
    }

    m_rendererID = program;
}


void Shader::SetInt(std::string_view name, int value)
{
    glProgramUniform1i(m_rendererID, GetUniformLocation(name), value);
}

void Shader::SetFloat(std::string_view name, float value)
{
    glProgramUniform1f(m_rendererID, GetUniformLocation(name), value);
}

void Shader::SetVec3(std::string_view name, const vec3& value)
{
    glProgramUniform3fv(m_rendererID, GetUniformLocation(name), 1, &value[0]);
}

void Shader::SetVec4(std::string_view name, const vec4& value)
{
    glProgramUniform4fv(m_rendererID, GetUniformLocation(name), 1, &value[0]);
}

void Shader::SetMat3(std::string_view name, const mat3& value)
{
    glProgramUniformMatrix3fv(m_rendererID, GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

void Shader::SetMat4(std::string_view name, const mat4& value)
{
    glProgramUniformMatrix4fv(m_rendererID, GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

// ===== HELPERS =====
GLint Shader::GetUniformLocation(std::string_view name)
{
    std::string nameStr(name);
    if (m_uniformLocCache.find(nameStr) != m_uniformLocCache.end())
        return m_uniformLocCache[nameStr];

    GLint location = glGetUniformLocation(m_rendererID, nameStr.c_str());
    if (location == -1)
        std::cerr << "Warning: Uniform '" << name << "' doesn't exist!" << std::endl;

    m_uniformLocCache[nameStr] = location;
    return location;
}

void Shader::CheckCompileErrors(GLuint object, std::string_view type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(object, 1024, nullptr, infoLog);
            std::cerr << "| ERROR::SHADER: Compile-time error\n" << infoLog << "\n" << std::endl;
        }
    }
    else
    {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(object, 1024, nullptr, infoLog);
            std::cerr << "| ERROR::SHADER: Link-time error\n" << infoLog << "\n" << std::endl;
        }
    }
}


} /* namespace BLR */