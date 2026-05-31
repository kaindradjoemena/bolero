// core/render_context.hpp

#pragma once

#include "types.hpp"
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
    GLuint GetTexture(const std::string& name) const 
    { 
        auto it = m_textures.find(name);
        return it != m_textures.end() ? it->second : 0; 
    }

    void SetMat4(const std::string& name, const mat4& mat)
    {
        m_matrices[name] = mat;
    }
    mat4 GetMat4(const std::string& name) const 
    { 
        auto it = m_matrices.find(name);
        return it != m_matrices.end() ? it->second : mat4(1.0f); 
    }

    void Clear()
    {
        m_textures.clear();
        m_matrices.clear();
    }

private:
    std::unordered_map<std::string, GLuint> m_textures;
    std::unordered_map<std::string, mat4> m_matrices;
};


} /* namespace blr::core */