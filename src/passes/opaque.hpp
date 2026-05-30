#pragma once

#include "core/types.hpp"
#include "renderer/pass.hpp"
#include "renderer/renderer.hpp"

#include "core/wrappers/framebuffer.hpp"
#include "core/shader.hpp"
#include "core/lights.hpp"
#include "shadow.hpp"

namespace blrc = blr::core;


class OpaquePass : public blrc::RenderPass
{
public:
    OpaquePass(const blrc::Ref<blrc::Shader>& lightShader, const blrc::Ref<ShadowPass>& shadowPass)
    : RenderPass("Main Opaque Pass")
    , m_lightShader(lightShader)
    , m_shadowPass(shadowPass)
    {
    }

    void Init() override
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        m_fbo = blrc::FrameBuffer::Create({ 1280, 720, { blrc::ImgFmt::RGBA8, blrc::ImgFmt::Depth32F } });
    }

    void Execute(blrc::Scene& scene) override
    {
        m_fbo->Bind(); 
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
        
        glViewport(0, 0, 1280, 720); 
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        blrc::Renderer::UpdateCameraUBO(*scene.GetCam());

        m_lightShader->SetMat4("u_LightSpaceMat", m_shadowPass->GetLightSpaceMat());
        m_lightShader->SetInt("u_depthMapTex", 10);
        glBindTextureUnit(10, m_shadowPass->GetDepthMap());

        blrc::Renderer::DrawQueue(nullptr);

        m_fbo->Unbind();
    }

    void Shutdown() override {}

    // Getter for the Post-Process Pass
    GLuint GetColorMap() const { return m_fbo->GetColorAttachmentID(0); }

private:
    blrc::Ref<blrc::FrameBuffer> m_fbo;
    blrc::Ref<blrc::Shader> m_lightShader;
    blrc::Ref<ShadowPass>   m_shadowPass;
};