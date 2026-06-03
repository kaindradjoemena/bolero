// passes/ibl_setup.hpp

#pragma once

#include <bolero.hpp>
#include "core/wrappers/cubemap.hpp"
#include "core/texture.hpp"


namespace blrc = blr::core;

class IBLSetupPass : public blrc::RenderPass
{
public:
    IBLSetupPass(const blrc::Ref<blrc::Shader>& eqToCubeShader, const blrc::Ref<blrc::Tex>& hdrMap)
    : RenderPass("IBL Setup Pass")
    , m_eqToCubeShader(eqToCubeShader)
    , m_hdrMap(hdrMap)
    {
    }

    void Init() override
    {
        blrc::TexSpec spec;
        spec.w = 512;
        spec.h = 512;
        spec.format = blrc::ImgFmt::RGB16F; 
        spec.generateMips = true;
        spec.minFilter = blrc::TexFilter::LinearMipmapLinear;
        spec.magFilter = blrc::TexFilter::Linear;
        spec.wrapS = blrc::TexWrap::ClampToEdge;
        spec.wrapT = blrc::TexWrap::ClampToEdge;
        
        m_envCubemap = blrc::Cubemap::Create(spec);

        glCreateFramebuffers(1, &m_fboID);
        glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_envCubemap->GetID(), 0);
    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        // note: comment/uncomment to optimize/renderdoc debug
        if (m_hasExecuted) 
        {
            renderCtx.SetTexture("u_EnvMap", m_envCubemap->GetID());
            return;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
        glViewport(0, 0, 512, 512);

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

        m_eqToCubeShader->Bind();
        for (size_t i = 0; i < 6; i++)
            m_eqToCubeShader->SetMat4("u_ViewProjMatrices[" + std::to_string(i) + "]", captureProj * captureViews[i]);

        m_eqToCubeShader->SetInt("u_EquirectangularMap", 0);
        m_hdrMap->Bind(0);

        blrc::Renderer::DrawCube(); 

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glGenerateTextureMipmap(m_envCubemap->GetID());

        m_hasExecuted = true;


        renderCtx.SetTexture("u_EnvMap", m_envCubemap->GetID());
    }

    void Shutdown() override
    {
        glDeleteFramebuffers(1, &m_fboID);
    }

private:
    blrc::Ref<blrc::Shader> m_eqToCubeShader;
    blrc::Ref<blrc::Tex> m_hdrMap;
    blrc::Ref<blrc::Cubemap> m_envCubemap;
    
    blrc::Ref<blrc::FrameBuffer> m_fbo;

    GLuint m_fboID{0};
    bool m_hasExecuted = false;
};