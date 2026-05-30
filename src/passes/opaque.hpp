#pragma once

#include <bolero.hpp>
#include "shadow.hpp"

namespace blrc = blr::core;


class OpaquePass : public blrc::RenderPass
{
public:
    OpaquePass(uint32_t initW, uint32_t initH, const blrc::Ref<blrc::Shader>& lightShader, const blrc::Ref<ShadowPass>& shadowPass)
    : RenderPass("Main Opaque Pass")
    , m_initW(initW)
    , m_initH(initH)
    , m_lightShader(lightShader)
    , m_shadowPass(shadowPass)
    {
    }

    void Init() override
    {
        m_fbo = blrc::FrameBuffer::Create({ m_initW, m_initH, { blrc::ImgFmt::RGBA8, blrc::ImgFmt::Depth32F} });
    }

    void Execute(blrc::Scene& scene) override
    {
        m_fbo->Bind(); 

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        blrc::Renderer::UpdateCameraUBO(*scene.GetCam());

        m_lightShader->SetMat4("u_LightSpaceMat", m_shadowPass->GetLightSpaceMat());
        
        m_lightShader->SetInt("u_depthMapTex", 10);
        glBindTextureUnit(10, m_shadowPass->GetDepthMap());

        blrc::Renderer::DrawQueue(nullptr);

        m_fbo->Unbind();
    }

    virtual void OnResize(uint32_t width, uint32_t height) override
    {
        m_fbo->Resize(width, height);
    }

    void Shutdown() override {}

    GLuint GetColorMap() const { return m_fbo->GetColorAttachmentID(0); }

private:
    uint32_t m_initW;
    uint32_t m_initH;

    blrc::Ref<blrc::FrameBuffer> m_fbo;
    blrc::Ref<blrc::Shader> m_lightShader;
    blrc::Ref<ShadowPass>   m_shadowPass;
};