#pragma once

#include <bolero.hpp>
#include "dir_shadow.hpp"
#include "spot_shadow.hpp"
#include "point_shadow.hpp"
#include "core/render_context.hpp"

namespace blrc = blr::core;


class OpaquePass : public blrc::RenderPass
{
public:
    OpaquePass(uint32_t initW, uint32_t initH, const blrc::Ref<blrc::Shader>& lightShader)
    : RenderPass("Main Opaque Pass")
    , m_initW(initW)
    , m_initH(initH)
    , m_lightShader(lightShader)
    {
    }

    void Init() override
    {
        m_fbo = blrc::FrameBuffer::Create({ m_initW, m_initH, { blrc::ImgFmt::RGBA16F, blrc::ImgFmt::Depth32F} });
    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        m_fbo->Bind(); 

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        
        glClearColor(0.03f, 0.03f, 0.03f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        blrc::Renderer::UpdateCameraUBO(*scene.GetCam());

        m_lightShader->SetMat4("u_DirLightSpaceMat", renderCtx.GetMat4("u_DirLightSpaceMat"));
        m_lightShader->SetMat4("u_SpotLightSpaceMat", renderCtx.GetMat4("u_SpotLightSpaceMat"));

        m_lightShader->SetInt("u_DirDepthMapTex", 10);
        glBindTextureUnit(10, renderCtx.GetTexture("u_DirDepthMapTex"));

        m_lightShader->SetInt("u_SpotDepthMapTex", 11);
        glBindTextureUnit(11, renderCtx.GetTexture("u_SpotDepthMapTex"));

        m_lightShader->SetInt("u_PointDepthMapTex", 12);
        glBindTextureUnit(12, renderCtx.GetTexture("u_PointDepthMapTex"));
        m_lightShader->SetFloat("u_PointFarPlane", scene.GetPointLights()[0].range);

        blrc::Renderer::DrawQueue(nullptr);

        m_fbo->Unbind();


        renderCtx.SetTexture("OPAQUE_PASS_TEX", m_fbo->GetColorAttachmentID(0));
    }

    virtual void OnResize(uint32_t width, uint32_t height) override
    {
        m_fbo->Resize(width, height);
    }

    void Shutdown() override {}

private:
    uint32_t m_initW;
    uint32_t m_initH;

    blrc::Ref<blrc::FrameBuffer> m_fbo;
    blrc::Ref<blrc::Shader> m_lightShader;
};