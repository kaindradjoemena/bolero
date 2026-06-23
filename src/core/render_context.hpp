// core/render_context.hpp

#pragma once

#include <variant>
#include <optional>
#include <unordered_map>
#include <string>

#include <glad/glad.h>
#include "utils/math.hpp"


namespace blr::core
{


using ContextValue = std::variant<GLuint, int, float, bool, vec2, vec3, vec4, mat3, mat4, std::string>;

enum class Lifetime { TRANSIENT, PERSISTENT };

class RenderContext
{
public:
    RenderContext() = default;
    ~RenderContext() = default;

    void Set(const std::string& name, ContextValue value, Lifetime lifetime = Lifetime::TRANSIENT)
    {
        if (lifetime == Lifetime::PERSISTENT)
            m_persistent[name] = std::move(value);
        else
            m_transient[name] = std::move(value);
    }

    template<typename T>
    std::optional<T> TryGet(const std::string& name) const
    {
        for (const auto* map : { &m_transient, &m_persistent })
        {
            auto it = map->find(name);
            if (it != map->end())
            {
                const T* val = std::get_if<T>(&it->second);
                return val ? std::optional<T>(*val) : std::nullopt;
            }
        }
        return std::nullopt;
    }

    template<typename T>
    T Get(const std::string& name, T fallback = T{}) const
    {
        return TryGet<T>(name).value_or(fallback);
    }

    bool Has(const std::string& name) const
    {
        return m_transient.find(name) != m_transient.end() || m_persistent.find(name) != m_persistent.end();
    }

    void ClearTransient()
    {
        m_transient.clear();
    }
    
    void Clear()
    {
        m_transient.clear();
        m_persistent.clear();
    }

private:
    std::unordered_map<std::string, ContextValue> m_transient;
    std::unordered_map<std::string, ContextValue> m_persistent;
};


} /* namespace blr::core */