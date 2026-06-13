// passes/post.hpp

#pragma once

#include <bolero.hpp>
#include "core/render_context.hpp"
#include "core/texture.hpp"
#include "core/asset_manager.hpp"


namespace blrc = blr::core;


class BrdfLutPass : public blrc::RenderPass
{
public:
    BrdfLutPass(blrc::AssetManager& assetManager, blrc::Ref<blrc::Shader>& brdfLutShader, uint32_t size = 512, uint32_t samples = 2048)
    : RenderPass("BRDF Pre Computation Pass")
    , m_assetManager(assetManager)
    , m_brdfLutShader(brdfLutShader)
    , m_size(size)
    , m_samples(samples)
    {
    }

    void Init() override
    {
        blrc::TexSpec spec;
        spec.w = m_size;
        spec.h = m_size;
        spec.format = blrc::ImgFmt::RG16F; 
        spec.generateMips = false;
        spec.wrapS = blrc::TexWrap::ClampToEdge;
        spec.wrapT = blrc::TexWrap::ClampToEdge;
        spec.minFilter = blrc::TexFilter::Linear;
        spec.magFilter = blrc::TexFilter::Linear;

        m_brdfTex = m_assetManager.CreateTex(spec);

        glCreateFramebuffers(1, &m_fboID);
        glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_brdfTex->GetID(), 0);
    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        // note: comment/uncomment to optimize/renderdoc debug
        if (m_hasExecuted)
        {
            renderCtx.Set("u_BrdfLut", m_brdfTex->GetID());
            return;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
        glViewport(0, 0, m_size, m_size); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_brdfLutShader->Bind();

        m_brdfLutShader->SetUInt("u_Samples", m_samples);

        blrc::Renderer::DrawFullscreenQuad();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_hasExecuted = true;
        renderCtx.Set("u_BrdfLut", m_brdfTex->GetID());
    }

    void Shutdown() override
    {
        glDeleteFramebuffers(1, &m_fboID);
    }

private:
    blrc::AssetManager& m_assetManager;
    blrc::Ref<blrc::Shader> m_brdfLutShader;
    blrc::Ref<blrc::Tex> m_brdfTex;

    blrc::Ref<blrc::FrameBuffer> m_fbo;
    uint32_t m_size;
    uint32_t m_samples;
    
    GLuint m_fboID{0};
    bool m_hasExecuted = false;
};