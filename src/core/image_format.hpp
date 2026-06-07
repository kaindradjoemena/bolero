#pragma once

#include <glad/glad.h>


namespace blr::core
{


enum class ImgFmt
{
    None = 0,
    R8,
    RG8,
    RG16F,
    RGB8,
    RGBA8,
    SRGB8,
    SRGBA8,
    RGBA16F,
    RGB16F,
    Depth32F,
    Depth24Stencil8
};

inline
GLenum ImgFmtToGLFmt(ImgFmt imgFmt)
{
    switch(imgFmt)
    {
        case ImgFmt::R8:              return GL_R8;
        case ImgFmt::RG8:             return GL_RG8;
        case ImgFmt::RG16F:           return GL_RG16F;
        case ImgFmt::RGB8:            return GL_RGB8;
        case ImgFmt::RGBA8:           return GL_RGBA8;
        case ImgFmt::RGB16F:          return GL_RGB16F;
        case ImgFmt::RGBA16F:         return GL_RGBA16F;
        case ImgFmt::Depth32F:        return GL_DEPTH_COMPONENT32F;
        case ImgFmt::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
        default:                      return GL_RGB8;
    }
}

inline
GLenum GetGLDataFmt(ImgFmt imgFmt)
{
    switch(imgFmt)
    {
        case ImgFmt::R8:              return GL_RED;
        case ImgFmt::RG8:             return GL_RG;
        case ImgFmt::RG16F:           return GL_RG;
        case ImgFmt::RGB8:            return GL_RGB;
        case ImgFmt::RGBA8:           return GL_RGBA;
        case ImgFmt::RGB16F:          return GL_RGB;
        case ImgFmt::RGBA16F:         return GL_RGBA;
        case ImgFmt::Depth32F:        return GL_DEPTH_COMPONENT;
        case ImgFmt::Depth24Stencil8: return GL_DEPTH_STENCIL;
        default:                      return GL_RGBA;
    }
}


} /* namespace blr::core */