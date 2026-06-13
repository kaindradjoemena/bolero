// core/wrappers/cubemap.cpp

#include "cubemap.hpp"


namespace blr::core
{


Cubemap::Cubemap(const TexSpec& spec)
: m_spec(spec)
{
    GLenum internalFmt = ImgFmtToGLFmt(m_spec.format);

    GLsizei mipLevels = 1;
    if (m_spec.generateMips)
    {
        if (m_spec.numMips > 0)
            mipLevels = m_spec.numMips;
        else
            mipLevels = static_cast<GLsizei>(std::floor(std::log2(std::max(m_spec.w, m_spec.h)))) + 1; // Auto full-chain
            m_spec.numMips = mipLevels;
    }

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_rendererID);
    
    glTextureStorage2D(m_rendererID, mipLevels, internalFmt, m_spec.w, m_spec.h);

    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, TextWrapToGLEnum(m_spec.wrapS));
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, TextWrapToGLEnum(m_spec.wrapT));
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_R, TextWrapToGLEnum(m_spec.wrapS)); 
    glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, TexFilterToGLEnum(m_spec.minFilter));
    glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, TexFilterToGLEnum(m_spec.magFilter));
}

Cubemap::~Cubemap()
{
    if (m_rendererID != 0)
        glDeleteTextures(1, &m_rendererID);
}

void Cubemap::Bind(uint32_t slot) const
{
    glBindTextureUnit(slot, m_rendererID);
}


} /* namespace blr::core */