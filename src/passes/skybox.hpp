// passes/skybox.hpp

#pragma once

#include <bolero.hpp>

namespace blrc = blr::core;

class SkyboxPass : public blrc::RenderPass
{
public:
    SkyboxPass(const blrc::Ref<blrc::Shader>& skyboxShader)
    : RenderPass("Skybox Pass")
    , m_skyboxShader(skyboxShader)
    {
    }

    void Init() override
    {
    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        GLuint sceneFbo = renderCtx.Get<GLuint>("SCENE_FBO_ID", 0);
        if (sceneFbo == 0)
            return;

        GLuint prefilterMap = renderCtx.Get<GLuint>("u_PrefilterMap", 0);
        if (prefilterMap == 0)
            return;

        uint32_t targetW = blrc::Renderer::GetViewportWidth();
        uint32_t targetH = blrc::Renderer::GetViewportHeight();
        uint32_t scale   = blrc::Renderer::GetRenderScale();
        glBindFramebuffer(GL_FRAMEBUFFER, sceneFbo);
        glViewport(0, 0, targetW * scale, targetH * scale);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        m_skyboxShader->Bind();

        glBindTextureUnit(6, renderCtx.Get<GLuint>("u_PrefilterMap"));
        m_skyboxShader->SetFloat("u_EnvironmentBlur",  renderCtx.Get<float>("u_EnvironmentBlur", 0.9f));
        m_skyboxShader->SetFloat("u_EnvironmentRot",   renderCtx.Get<float>("u_EnvironmentRot", 0.0f));
        m_skyboxShader->SetFloat("u_EnvironmentPower", renderCtx.Get<float>("u_EnvironmentPower", 1.0f));

        blrc::Renderer::DrawCube();

        glDepthFunc(GL_LESS);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    virtual void OnWindowResize(uint32_t width, uint32_t height) override
    {
    }

    void Shutdown() override
    {
    }

private:
    blrc::Ref<blrc::Shader> m_skyboxShader;
};