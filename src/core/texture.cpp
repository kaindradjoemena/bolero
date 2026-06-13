// core/texture.cpp

#include "texture.hpp"

#include <stb_image.h>

#include <cmath>


namespace blr::core
{


Tex::Tex(const std::filesystem::path& texPath, const TexSpec& texSpec)
: m_texPath(texPath), m_texSpec(texSpec)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    
    void* imgData = nullptr;
    bool isHDR = stbi_is_hdr(texPath.string().c_str());

    if (isHDR)
    {
        imgData = stbi_loadf(texPath.string().c_str(), &width, &height, &channels, 0);
        if (m_texSpec.format == ImgFmt::None)
            m_texSpec.format = (channels == 4) ? ImgFmt::RGBA16F : ImgFmt::RGB16F;
    }
    else
    {
        imgData = stbi_load(texPath.string().c_str(), &width, &height, &channels, 0);
        if (m_texSpec.format == ImgFmt::None)
        {
            if (channels == 4)      m_texSpec.format = ImgFmt::RGBA8;
            else if (channels == 3) m_texSpec.format = ImgFmt::RGB8;
            else if (channels == 2) m_texSpec.format = ImgFmt::RG8;
            else if (channels == 1) m_texSpec.format = ImgFmt::R8;
        }
    }

    if (!imgData)
    {
        std::cerr << "Failed to load texture: " << texPath << std::endl;
        std::cerr << stbi_failure_reason() << std::endl;
        return;
    }

    m_texSpec.w = width;
    m_texSpec.h = height;

    GLenum internalFmt = ImgFmtToGLFmt(m_texSpec.format);
    GLenum dataFmt     = GetGLDataFmt(m_texSpec.format);
    GLenum dataType    = isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE;

    GLsizei mipLevels = 1;
    if (m_texSpec.generateMips)
        mipLevels = static_cast<GLsizei>(std::floor(std::log2(std::max(width, height)))) + 1;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
    glTextureStorage2D(m_rendererID, mipLevels, internalFmt, width, height);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTextureSubImage2D(m_rendererID, 0, 0, 0, width, height, dataFmt, dataType, imgData);
    if (m_texSpec.generateMips)
        glGenerateTextureMipmap(m_rendererID);

    stbi_image_free(imgData);

    if (channels == 1)
    {
        glTextureParameteri(m_rendererID, GL_TEXTURE_SWIZZLE_G, GL_RED);
        glTextureParameteri(m_rendererID, GL_TEXTURE_SWIZZLE_B, GL_RED);
    }

    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S,     TextWrapToGLEnum(m_texSpec.wrapS));
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T,     TextWrapToGLEnum(m_texSpec.wrapT));
    glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, TexFilterToGLEnum(m_texSpec.minFilter));
    glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, TexFilterToGLEnum(m_texSpec.magFilter));
}

Tex::Tex(const TexSpec& spec)
    : m_texSpec(spec)
{
    GLenum internalFmt = ImgFmtToGLFmt(m_texSpec.format);
    
    GLsizei mipLevels = 1;
    if (m_texSpec.generateMips)
        mipLevels = static_cast<GLsizei>(std::floor(std::log2(std::max(m_texSpec.w, m_texSpec.h)))) + 1;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
    glTextureStorage2D(m_rendererID, mipLevels, internalFmt, m_texSpec.w, m_texSpec.h);

    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S,     TextWrapToGLEnum(m_texSpec.wrapS));
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T,     TextWrapToGLEnum(m_texSpec.wrapT));
    glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, TexFilterToGLEnum(m_texSpec.minFilter));
    glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, TexFilterToGLEnum(m_texSpec.magFilter));
}

Tex::~Tex()
{
    if (m_rendererID != 0)
        glDeleteTextures(1, &m_rendererID);
}

void Tex::Bind(uint32_t slot) const
{
    glBindTextureUnit(slot, m_rendererID);
}


} /* namespace blr::core */