#pragma once

#include <bolero.hpp>
#include "passes/opaque.hpp"
#include "core/render_context.hpp"

namespace blrc = blr::core;


class PostPass : public blrc::RenderPass
{
public:
    PostPass(uint32_t width, uint32_t height, const blrc::Ref<blrc::Shader>& postShader, const blrc::Ref<OpaquePass>& opaquePass)
    : RenderPass("Post Pass")
    , m_windowW(width)
    , m_windowH(height)
    , m_postShader(postShader)
    , m_opaquePass(opaquePass)
    {
    }

    void Init() override
    {
    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        glDisable(GL_DEPTH_TEST);     
        glDisable(GL_CULL_FACE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_windowW, m_windowH); 
        glClearColor(0.03f, 0.03f, 0.03f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_postShader->Bind();

        m_postShader->SetInt("u_ScreenTexture", 15);
        glBindTextureUnit(15, renderCtx.GetTexture("OPAQUE_PASS_TEX"));

        blrc::Renderer::DrawFullscreenQuad();

        m_postShader->Unbind();
    }

    virtual void OnResize(uint32_t width, uint32_t height) override
    {
        m_windowW = width;
        m_windowH = height;
    }

    void Shutdown() override {}

private:
    uint32_t m_windowW;
    uint32_t m_windowH;

    blrc::Ref<blrc::Shader> m_postShader;
    blrc::Ref<OpaquePass> m_opaquePass;
};