// core/render_context.hpp

#pragma once

#include <unordered_map>
#include <string>


namespace blr::core
{


class RenderContext
{
public:
    RenderContext() = default;
    ~RenderContext() = default;

    void SetTexture(const std::string& name, GLuint id)
    {
        m_textures[name] = id;
    }
    GLuint GetTexture(const std::string& name, int fallback = 0) const 
    { 
        auto it = m_textures.find(name);
        return it != m_textures.end() ? it->second : fallback; 
    }

    void SetInt(const std::string& name, int x)
    {
        m_nums[name] = x;
    }
    int GetInt(const std::string& name, int fallback = 0)
    {
        auto it = m_nums.find(name);
        return it != m_nums.end() ? it->second : fallback; 
    }

    void SetMat4(const std::string& name, const mat4& mat)
    {
        m_matrices[name] = mat;
    }
    mat4 GetMat4(const std::string& name, const mat4& fallback = mat4(1.0f)) const 
    { 
        auto it = m_matrices.find(name);
        return it != m_matrices.end() ? it->second : fallback; 
    }

    void Clear()
    {
        m_textures.clear();
        m_matrices.clear();
    }

private:
    std::unordered_map<std::string, GLuint> m_textures;
    std::unordered_map<std::string, mat4> m_matrices;
    std::unordered_map<std::string, int> m_nums;
};


} /* namespace blr::core */