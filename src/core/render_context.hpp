// core/render_context.hpp

#pragma once

#include <variant>
#include <optional>
#include <cassert>

#include <unordered_map>
#include <string>


namespace blr::core
{


using ContextValue = std::variant<GLuint, int, float, bool, vec4, mat4>;

class RenderContext
{
public:
    RenderContext() = default;
    ~RenderContext() = default;

    void Set(const std::string& name, ContextValue value)
    {
        m_entries[name] = std::move(value);
    }

    template<typename T>
    std::optional<T> TryGet(const std::string& name) const
    {
        auto it = m_entries.find(name);
        if (it == m_entries.end())
            return std::nullopt;

        const T* val = std::get_if<T>(&it->second);
        return val ? std::optional<T>(*val) : std::nullopt;
    }

    template<typename T>
    T Get(const std::string& name, T fallback = T{}) const
    {
        return TryGet<T>(name).value_or(fallback);
    }

    bool Has(const std::string& name) const
    {
        return m_entries.find(name) != m_entries.end();
    }

    void Clear()
    {
        m_entries.clear();
    }

private:
    std::unordered_map<std::string, ContextValue> m_entries;
};


} /* namespace blr::core */