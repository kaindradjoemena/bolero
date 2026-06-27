// core/texture.hpp

#pragma once

#include "resource.hpp"

#include "utils/base.hpp"
#include "image_format.hpp"

#include <filesystem>
#include <iostream>


namespace blr::core
{


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
    uint32_t numMips    = 0;
    ImgFmt format       = ImgFmt::None;
    TexWrap wrapS       = TexWrap::Repeat;
    TexWrap wrapT       = TexWrap::Repeat;
    TexFilter minFilter = TexFilter::LinearMipmapLinear;
    TexFilter magFilter = TexFilter::Linear;
};


class Tex : public Resource
{
public:
    ~Tex();

    // Prevent copying
    Tex(const Tex&) = delete;
    Tex& operator=(const Tex&) = delete;
    
    // Allow moving
    Tex(Tex&& other) noexcept;
    Tex& operator=(Tex&& other) noexcept;

    uint32_t GetID() const { return m_rendererID; }
    uint32_t GetWidth() const { return m_texSpec.w; }
    uint32_t GetHeight() const { return m_texSpec.h; }
    const TexSpec& GetTexSpec() const { return m_texSpec; }
    const std::filesystem::path& GetPath() const { return m_texPath; }

    void Bind(uint32_t slot) const;

private:
    GLuint m_rendererID{0};
    std::filesystem::path m_texPath;
    TexSpec m_texSpec;

// Construction must be called by the AssetManager class through the Create method
friend class AssetManager;
protected:
    Tex(const std::filesystem::path& texPath, const TexSpec& texSpec = TexSpec{});
    static Ref<Tex> Create(const std::filesystem::path& texPath, const TexSpec& texSpec = TexSpec{})
    {
        return std::shared_ptr<Tex>(new Tex(texPath, texSpec));
    }
    
    Tex(const TexSpec& texSpec);
    static Ref<Tex> Create(const TexSpec& texSpec)
    {
        return std::shared_ptr<Tex>(new Tex(texSpec));
    }
};


} /* namespace blr::core */