// core/wrappers/cubemap.hpp

#pragma once

#include <glad/glad.h>

#include "utils/base.hpp"
#include "core/texture.hpp"

#include <cmath>
#include <cstdint>
#include <algorithm>


namespace blr::core
{


class Cubemap
{
public:
    static Ref<Cubemap> Create(const TexSpec& spec)
    {
        return std::shared_ptr<Cubemap>(new Cubemap(spec));
    }

    ~Cubemap();

    // Prevent copying
    Cubemap(const Cubemap&) = delete;
    Cubemap& operator=(const Cubemap&) = delete;

    // Allow moving
    Cubemap(Cubemap&& other) = default;
    Cubemap& operator=(Cubemap&& other) = default;


    GLuint GetID() const { return m_rendererID; }
    uint32_t GetWidth() const { return m_spec.w; }
    uint32_t GetHeight() const { return m_spec.h; }
    const TexSpec& GetSpec() const { return m_spec; }

    void Bind(uint32_t slot) const;

private:
    Cubemap(const TexSpec& spec);

    GLuint m_rendererID{0};
    TexSpec m_spec;
};


} /* namespace blr::core */