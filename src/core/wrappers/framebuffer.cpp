// core/wrappers/framebuffer.cpp

#include "framebuffer.hpp"

#include <iostream>


namespace blr::core
{

static bool IsDepthFormat(ImgFmt format)
{
    return format == ImgFmt::Depth32F || format == ImgFmt::Depth24Stencil8;
}

FrameBuffer::FrameBuffer(const FBSpec& spec)
: m_spec(spec)
{
    for (auto& attachSpec : m_spec.attachments)
    {
        if (IsDepthFormat(attachSpec.format))
        {
            m_depthSpec = attachSpec;
        }
        else
        {
            m_colorSpecs.push_back(attachSpec);
        }
    }

    Invalidate();
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
: m_rendererID(other.m_rendererID)
, m_spec(std::move(other.m_spec))
, m_colorSpecs(std::move(other.m_colorSpecs))
, m_depthSpec(std::move(other.m_depthSpec))
, m_colorAttachments(std::move(other.m_colorAttachments))
, m_depthAttachment(other.m_depthAttachment)
{
    other.m_rendererID = 0;
    other.m_colorAttachments.clear();
    other.m_depthAttachment = 0;
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept
{
    if (this != &other)
    {
        if (m_rendererID != 0)
            glDeleteTextures(1, &m_rendererID);
        
        m_rendererID = other.m_rendererID;
        m_spec = std::move(other.m_spec);
        m_colorSpecs = std::move(other.m_colorSpecs);
        m_depthSpec = std::move(other.m_depthSpec);
        m_colorAttachments = std::move(other.m_colorAttachments);
        m_depthAttachment = other.m_depthAttachment;

        other.m_rendererID = 0;
        other.m_colorAttachments.clear();
        other.m_depthAttachment = 0;
    }

    return *this;
}

FrameBuffer::~FrameBuffer()
{
    if (m_rendererID != 0)
    {
        glDeleteFramebuffers(1, &m_rendererID);
        glDeleteTextures(m_colorAttachments.size(), m_colorAttachments.data());
        glDeleteTextures(1, &m_depthAttachment);
    }
}

void FrameBuffer::Invalidate()
{
    if (m_rendererID)
    {
        glDeleteFramebuffers(1, &m_rendererID);
        glDeleteTextures(m_colorAttachments.size(), m_colorAttachments.data());
        glDeleteTextures(1, &m_depthAttachment);
        m_colorAttachments.clear();
        m_depthAttachment = 0;
    }

    glCreateFramebuffers(1, &m_rendererID);

    // Color
    if (!m_colorSpecs.empty())
    {
        m_colorAttachments.resize(m_colorSpecs.size());
        glCreateTextures(GL_TEXTURE_2D, m_colorAttachments.size(), m_colorAttachments.data());

        for (size_t i = 0; i < m_colorAttachments.size(); i++)
        {
            GLenum internalFormat = ImgFmtToGLFmt(m_colorSpecs[i].format);

            glTextureStorage2D(m_colorAttachments[i], 1, internalFormat, m_spec.w, m_spec.h);
            
            glTextureParameteri(m_colorAttachments[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_colorAttachments[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(m_colorAttachments[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_colorAttachments[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glNamedFramebufferTexture(m_rendererID, GL_COLOR_ATTACHMENT0 + i, m_colorAttachments[i], 0);
        }
    }

    // Depth
    if (m_depthSpec.format != ImgFmt::None)
    {
        GLenum internalFormat = ImgFmtToGLFmt(m_depthSpec.format);
        GLenum attachmentType = (m_depthSpec.format == ImgFmt::Depth24Stencil8)
                                ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;

        // For cubemaps
        if (m_depthSpec.isCubemap)
        {
            glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_depthAttachment);
            
            glTextureStorage2D(m_depthAttachment, 1, internalFormat, m_spec.w, m_spec.h);
            
            glTextureParameteri(m_depthAttachment, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(m_depthAttachment, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTextureParameteri(m_depthAttachment, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_depthAttachment, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_depthAttachment, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glNamedFramebufferTexture(m_rendererID, attachmentType, m_depthAttachment, 0);
        }
        else
        {
            glCreateTextures(GL_TEXTURE_2D, 1, &m_depthAttachment);
            glTextureStorage2D(m_depthAttachment, 1, internalFormat, m_spec.w, m_spec.h);
            
            glTextureParameteri(m_depthAttachment, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(m_depthAttachment, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTextureParameteri(m_depthAttachment, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTextureParameteri(m_depthAttachment, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTextureParameterfv(m_depthAttachment, GL_TEXTURE_BORDER_COLOR, borderColor);

            glNamedFramebufferTexture(m_rendererID, attachmentType, m_depthAttachment, 0);
        }
    }

    if (m_colorAttachments.empty())
    {
        glNamedFramebufferDrawBuffer(m_rendererID, GL_NONE);
        glNamedFramebufferReadBuffer(m_rendererID, GL_NONE);
    }
    else
    {
        std::vector<GLenum> buffers;
        for (size_t i = 0; i < m_colorAttachments.size(); i++)
            buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
        
        glNamedFramebufferDrawBuffers(m_rendererID, buffers.size(), buffers.data());
    }

    if (glCheckNamedFramebufferStatus(m_rendererID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Framebuffer is incomplete!\n";
}

void FrameBuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);
    glViewport(0, 0, m_spec.w, m_spec.h);
}

void FrameBuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Resize(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0 || (m_spec.w == width && m_spec.h == height))
        return;

    m_spec.w = width;
    m_spec.h = height;
    Invalidate();
}


} /* namespace blr::core */