// passes/post.hpp

#pragma once

#include <bolero.hpp>

namespace blrc = blr::core;


class PostPass : public blrc::RenderPass
{
public:
    PostPass(const blrc::Ref<blrc::Shader>& postShader)
    : RenderPass("Post Pass")
    , m_postShader(postShader)
    {
    }

    void Init() override
    {
        m_renderW = blrc::Renderer::GetViewportWidth();
        m_renderH = blrc::Renderer::GetViewportHeight();
        m_fbo = blrc::FrameBuffer::Create({ m_renderW, m_renderH, { blrc::ImgFmt::RGBA16F} });
    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        uint32_t targetW = blrc::Renderer::GetViewportWidth();
        uint32_t targetH = blrc::Renderer::GetViewportHeight();
        if (targetW != m_renderW || targetH != m_renderH)
        {
            m_renderW = targetW;
            m_renderH = targetH;
            m_fbo->Resize(m_renderW, m_renderH);
        }
        m_fbo->Bind();

        glDisable(GL_DEPTH_TEST);     
        glDisable(GL_CULL_FACE);

        glClearColor(0.03f, 0.03f, 0.03f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_postShader->Bind();

        m_postShader->SetInt("u_ScreenTexture", 20);
        glBindTextureUnit(20, renderCtx.Get<GLuint>("RESOLVED_TEX"));

        m_postShader->SetFloat("u_Exposure", renderCtx.Get<float>("u_Exposure", 1.0f));

        blrc::Renderer::DrawFullscreenQuad();

        m_postShader->Unbind();


        m_fbo->Unbind();


        renderCtx.Set("POST_PASS_TEX", m_fbo->GetColorAttachmentID(0));
    }

    virtual void OnWindowResize(uint32_t width, uint32_t height) override
    {
    }

    void Shutdown() override {}

private:
    uint32_t m_renderW;
    uint32_t m_renderH;

    blrc::Ref<blrc::Shader> m_postShader;
    blrc::Ref<blrc::FrameBuffer> m_fbo;
};