// passes/irradiance.hpp

#pragma once

#include <bolero.hpp>
#include "core/wrappers/cubemap.hpp"
#include "core/texture.hpp"


namespace blrc = blr::core;

class IrradiancePass : public blrc::RenderPass
{
public:
    IrradiancePass(const blrc::Ref<blrc::Shader>& convolutionShader, uint32_t size = 64)
    : RenderPass("Irradiance Pass")
    , m_convolutionShader(convolutionShader)
    , m_size(size)
    {
    }

    void Init() override
    {
        blrc::TexSpec spec;
        spec.w = m_size;
        spec.h = m_size;
        spec.format = blrc::ImgFmt::RGB16F; 
        spec.generateMips = false;
        spec.minFilter = blrc::TexFilter::LinearMipmapLinear;
        spec.magFilter = blrc::TexFilter::Linear;
        spec.wrapS = blrc::TexWrap::ClampToEdge;
        spec.wrapT = blrc::TexWrap::ClampToEdge;
        
        m_irradianceMap = blrc::Cubemap::Create(spec);

        glCreateFramebuffers(1, &m_fboID);
        glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_irradianceMap->GetID(), 0);

    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        // note: comment/uncomment to optimize/renderdoc debug
        if (m_hasExecuted) 
        {
            renderCtx.Set("u_IrradianceMap", m_irradianceMap->GetID());
            return;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
        glViewport(0, 0, m_size, m_size);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        blrc::mat4 captureProj = blrc::Perspective(blrc::DegToRad(90.0f), 1.0f, 0.1f, 10.0f);
        blrc::mat4 captureViews[] = {
                blrc::LookAt(blrc::vec3(0.0f), blrc::vec3( 1.0f,  0.0f,  0.0f), blrc::vec3(0.0f, -1.0f,  0.0f)),
                blrc::LookAt(blrc::vec3(0.0f), blrc::vec3(-1.0f,  0.0f,  0.0f), blrc::vec3(0.0f, -1.0f,  0.0f)),
                blrc::LookAt(blrc::vec3(0.0f), blrc::vec3( 0.0f,  1.0f,  0.0f), blrc::vec3(0.0f,  0.0f,  1.0f)),
                blrc::LookAt(blrc::vec3(0.0f), blrc::vec3( 0.0f, -1.0f,  0.0f), blrc::vec3(0.0f,  0.0f, -1.0f)),
                blrc::LookAt(blrc::vec3(0.0f), blrc::vec3( 0.0f,  0.0f,  1.0f), blrc::vec3(0.0f, -1.0f,  0.0f)),
                blrc::LookAt(blrc::vec3(0.0f), blrc::vec3( 0.0f,  0.0f, -1.0f), blrc::vec3(0.0f, -1.0f,  0.0f))
            };

        m_convolutionShader->Bind();
        for (size_t i = 0; i < 6; i++)
            m_convolutionShader->SetMat4("u_ViewProjMatrices[" + std::to_string(i) + "]", captureProj * captureViews[i]);

        m_convolutionShader->SetInt("u_EnvMap", 1);
        glBindTextureUnit(1, renderCtx.Get<GLuint>("u_EnvMap"));

        blrc::Renderer::DrawCube();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_hasExecuted = true;


        renderCtx.Set("u_IrradianceMap", m_irradianceMap->GetID());
    }

    void Shutdown() override
    {
        glDeleteFramebuffers(1, &m_fboID);
    }

private:
    blrc::Ref<blrc::Shader> m_convolutionShader;
    blrc::Ref<blrc::Cubemap> m_irradianceMap;
    
    blrc::Ref<blrc::FrameBuffer> m_fbo;
    uint32_t m_size;

    GLuint m_fboID{0};
    bool m_hasExecuted = false;
};