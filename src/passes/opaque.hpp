#pragma once

#include <bolero.hpp>

namespace blrc = blr::core;

class OpaquePass : public blrc::RenderPass
{
public:
    OpaquePass(const blrc::Ref<blrc::Shader>& lightShader)
    : RenderPass("Main Opaque Pass")
    , m_lightShader(lightShader)
    {
    }

    void Init() override
    {
        m_renderW = blrc::Renderer::GetViewportWidth();
        m_renderH = blrc::Renderer::GetViewportHeight();
        m_renderScale = blrc::Renderer::GetRenderScale();
        m_fbo = blrc::FrameBuffer::Create({ m_renderW * m_renderScale, m_renderH * m_renderScale, { blrc::ImgFmt::RGBA16F, blrc::ImgFmt::Depth32F} });
    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        uint32_t targetW = blrc::Renderer::GetViewportWidth();
        uint32_t targetH = blrc::Renderer::GetViewportHeight();
        uint32_t scale   = blrc::Renderer::GetRenderScale();
        if (scale != m_renderScale || targetW != m_renderW || targetH != m_renderH) 
        {
            m_renderScale = scale;
            m_renderW = targetW;
            m_renderH = targetH;
            m_fbo->Resize(m_renderW * m_renderScale, m_renderH * m_renderScale);
        }
        m_fbo->Bind(); 

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        
        // Background Color
        blrc::vec4 backgroundCol = renderCtx.Get<blrc::vec4>("u_BackgroundColor", blrc::vec4(0.0f, 0.0f, 0.0f, 1.0));
        glClearColor(backgroundCol.r, backgroundCol.g, backgroundCol.b, backgroundCol.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        blrc::Renderer::UpdateCameraUBO(*scene.GetCam());

        m_lightShader->Bind(); 

        // IBL Maps (Slots 5, 6, 7)
        GLuint irradianceMap = renderCtx.Get<GLuint>("u_IrradianceMap");
        GLuint prefilterMap  = renderCtx.Get<GLuint>("u_PrefilterMap");

        bool useIBL = (irradianceMap != 0 && prefilterMap != 0);
        if (useIBL)
        {
            glBindTextureUnit(5, irradianceMap);
            glBindTextureUnit(6, prefilterMap);
            glBindTextureUnit(7, renderCtx.Get<GLuint>("u_BrdfLut"));
        }
        m_lightShader->SetFloat("u_EnvironmentRot", renderCtx.Get<float>("u_EnvironmentRot", 0.0f));
        m_lightShader->SetFloat("u_EnvironmentPower", renderCtx.Get<float>("u_EnvironmentPower", 1.0f));
        m_lightShader->SetBool("u_UseIBL", useIBL ? true : false);


        // Directional Shadows (Slots 10 - 13)
        int numDirShadows = renderCtx.Get<int>("u_NumDirShadows"); 
        numDirShadows = std::min(numDirShadows, 4);
        for (int i = 0; i < numDirShadows; i++)
        {
            glBindTextureUnit(10 + i, renderCtx.Get<GLuint>("u_DirDepthMapTex_" + std::to_string(i)));

            m_lightShader->SetMat4("u_DirLightSpaceMat[" + std::to_string(i) + "]", renderCtx.Get<blrc::mat4>("u_DirLightSpaceMat_" + std::to_string(i)));
        }
        // Spot Shadows (Slots 14 - 17)
        int numSpotShadows = renderCtx.Get<int>("u_NumSpotShadows");
        numSpotShadows = std::min(numSpotShadows, 4);
        for (int i = 0; i < numSpotShadows; i++)
        {
            glBindTextureUnit(14 + i, renderCtx.Get<GLuint>("u_SpotDepthMapTex_" + std::to_string(i)));

            m_lightShader->SetMat4("u_SpotLightSpaceMat[" + std::to_string(i) + "]", renderCtx.Get<blrc::mat4>("u_SpotLightSpaceMat_" + std::to_string(i)));
        }
        // Point Shadows (Slots 18 - 21)
        int numPointShadows = renderCtx.Get<int>("u_NumPointShadows");
        numPointShadows = std::min(numPointShadows, 4);
        for (int i = 0; i < numPointShadows; i++)
        {
            glBindTextureUnit(18 + i, renderCtx.Get<GLuint>("u_PointDepthMapTex_" + std::to_string(i)));
        }

        // Draw Opaque geometry
        blrc::Renderer::DrawQueue(blrc::RenderQueueType::OPAQUE, nullptr);


        m_fbo->Unbind();


        renderCtx.Set("OPAQUE_PASS_TEX", m_fbo->GetColorAttachmentID(0));
        renderCtx.Set("SCENE_FBO_ID", m_fbo->GetRendererID());
    }

    virtual void OnWindowResize(uint32_t width, uint32_t height) override
    {
    }

    void Shutdown() override {}

private:
    uint32_t m_renderW;
    uint32_t m_renderH;
    uint32_t m_renderScale;

    blrc::Ref<blrc::FrameBuffer> m_fbo;
    blrc::Ref<blrc::Shader> m_lightShader;
};