// core/texture.hpp

#pragma once

#include "types.hpp"

#include <filesystem>
#include <iostream>


namespace blr::core
{


class Tex
{
public:
    ~Tex();

    // Prevent copying
    Tex(const Tex&) = delete;
    Tex& operator=(const Tex&) = delete;
    
    // Allow moving
    Tex(Tex&& other) = default;
    Tex& operator=(Tex&& other) = default;

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