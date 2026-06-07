// core/wrappers/framebuffer.hpp

#pragma once

#include "utils/base.hpp"
#include "core/image_format.hpp"

#include <vector>


namespace blr::core
{


struct FBAttachmentSpec
{
    ImgFmt format  = ImgFmt::None;
    bool isCubemap = false;

    FBAttachmentSpec() = default;
    FBAttachmentSpec(ImgFmt fmt, bool cubemap = false)
    : format(fmt), isCubemap(cubemap)
    {
    } 
};
struct FBSpec
{
    uint32_t w = 0;
    uint32_t h = 0;
    std::vector<FBAttachmentSpec> attachments;
};


class FrameBuffer
{
public:
    static Ref<FrameBuffer> Create(const FBSpec& spec)
    {
        return std::shared_ptr<FrameBuffer>(new FrameBuffer(spec));
    }

    ~FrameBuffer();

    // Prevent copying
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    // Allow moving
    FrameBuffer(FrameBuffer&& other) = default;
    FrameBuffer& operator=(FrameBuffer&& other) = default;


    void Bind() const;
    void Unbind() const;

    void Resize(uint32_t width, uint32_t height);

    GLuint GetColorAttachmentID(uint32_t index = 0) const { return m_colorAttachments[index]; }
    GLuint GetDepthAttachmentID() const { return m_depthAttachment; }
    
    const FBSpec& GetSpec() const { return m_spec; }

private:
    FrameBuffer(const FBSpec& spec);

    void Invalidate();

    GLuint m_rendererID{0};
    FBSpec m_spec;
    std::vector<FBAttachmentSpec> m_colorSpecs;
    FBAttachmentSpec m_depthSpec{ImgFmt::None};

    std::vector<GLuint> m_colorAttachments;
    GLuint m_depthAttachment{0};
};


} /* namespace blr::core */