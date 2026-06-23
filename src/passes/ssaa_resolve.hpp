// passes/ssaa_resolve.hpp

#pragma once

#include <bolero.hpp>

namespace blrc = blr::core;

class SSAAResolvePass : public blrc::RenderPass
{
public:
    SSAAResolvePass()
    : RenderPass("SSAA Resolve Pass")
    {
    }

    void Init() override
    {
        m_renderW = blrc::Renderer::GetViewportWidth();
        m_renderH = blrc::Renderer::GetViewportHeight();
        m_resolveFbo = blrc::FrameBuffer::Create({ m_renderW, m_renderH, { blrc::ImgFmt::RGBA16F } });
    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        GLuint sceneFbo = renderCtx.Get<GLuint>("SCENE_FBO_ID", 0);
        if (sceneFbo == 0)
            return;

        uint32_t targetW = blrc::Renderer::GetViewportWidth();
        uint32_t targetH = blrc::Renderer::GetViewportHeight();
        uint32_t scale   = blrc::Renderer::GetRenderScale();
        if (targetW != m_renderW || targetH != m_renderH)
        {
            m_renderW = targetW;
            m_renderH = targetH;
            m_resolveFbo->Resize(m_renderW, m_renderH);
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, sceneFbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resolveFbo->GetRendererID());

        // Downsample
        glBlitFramebuffer(0, 0, m_renderW * scale, m_renderH * scale,
                            0, 0, m_renderW, m_renderH,
                            GL_COLOR_BUFFER_BIT, GL_LINEAR);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        
        renderCtx.Set("RESOLVED_TEX", m_resolveFbo->GetColorAttachmentID(0));
    }

    void Shutdown() override {}

private:
    uint32_t m_renderW;
    uint32_t m_renderH;

    blrc::Ref<blrc::FrameBuffer> m_resolveFbo;
};