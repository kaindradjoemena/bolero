#pragma once

#include <bolero.hpp>
#include "dir_shadow.hpp"
#include "spot_shadow.hpp"
#include "point_shadow.hpp"

namespace blrc = blr::core;


class OpaquePass : public blrc::RenderPass
{
public:
    OpaquePass(uint32_t initW, uint32_t initH, const blrc::Ref<blrc::Shader>& lightShader
              , const blrc::Ref<DirShadowPass>& dirShadowPass
              , const blrc::Ref<SpotShadowPass>& spotShadowPass
              , const blrc::Ref<PointShadowPass>& pointShadowPass)
    : RenderPass("Main Opaque Pass")
    , m_initW(initW)
    , m_initH(initH)
    , m_lightShader(lightShader)
    , m_dirShadowPass(dirShadowPass)
    , m_spotShadowPass(spotShadowPass)
    , m_pointShadowPass(pointShadowPass)
    {
    }

    void Init() override
    {
        m_fbo = blrc::FrameBuffer::Create({ m_initW, m_initH, { blrc::ImgFmt::RGBA16F, blrc::ImgFmt::Depth32F} });
    }

    void Execute(blrc::Scene& scene) override
    {
        m_fbo->Bind(); 

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        
        glClearColor(0.03f, 0.03f, 0.03f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        blrc::Renderer::UpdateCameraUBO(*scene.GetCam());

        m_lightShader->SetMat4("u_DirLightSpaceMat", m_dirShadowPass->GetLightSpaceMat());
        m_lightShader->SetMat4("u_SpotLightSpaceMat", m_spotShadowPass->GetLightSpaceMat());

        m_lightShader->SetInt("u_DirDepthMapTex", 10);
        glBindTextureUnit(10, m_dirShadowPass->GetDepthMap());

        m_lightShader->SetInt("u_SpotDepthMapTex", 11);
        glBindTextureUnit(11, m_spotShadowPass->GetDepthMap());

        m_lightShader->SetInt("u_PointDepthMapTex", 12);
        glBindTextureUnit(12, m_pointShadowPass->GetDepthMap());
        m_lightShader->SetFloat("u_PointFarPlane", scene.GetPointLights()[0].range);

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

    blrc::Ref<DirShadowPass>   m_dirShadowPass;
    blrc::Ref<SpotShadowPass>  m_spotShadowPass;
    blrc::Ref<PointShadowPass> m_pointShadowPass;
};