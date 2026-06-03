// passes/prefilter.hpp

#pragma once

#include <bolero.hpp>
#include "core/wrappers/cubemap.hpp"
#include "core/texture.hpp"


namespace blrc = blr::core;

class PrefilterPass : public blrc::RenderPass
{
public:
    PrefilterPass(const blrc::Ref<blrc::Shader>& prefilterShader)
    : RenderPass("Prefilter Pass")
    , m_prefilterShader(prefilterShader)
    {
    }

    void Init() override
    {
        blrc::TexSpec spec;
        spec.w = 128;
        spec.h = 128;
        spec.format = blrc::ImgFmt::RGB16F; 
        spec.generateMips = true;
        spec.numMips = 5;
        spec.minFilter = blrc::TexFilter::LinearMipmapLinear;
        spec.magFilter = blrc::TexFilter::Linear;
        spec.wrapS = blrc::TexWrap::ClampToEdge;
        spec.wrapT = blrc::TexWrap::ClampToEdge;
        
        m_prefilteredMap = blrc::Cubemap::Create(spec);

        glCreateFramebuffers(1, &m_fboID);
        glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_prefilteredMap->GetID(), 0);

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        // note: comment/uncomment to optimize/renderdoc debug
        // if (m_hasExecuted) 
        // {
        //     renderCtx.SetTexture("u_PrefilterMap", m_prefilteredMap->GetID());
        //     return;
        // }

        glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

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

        m_prefilterShader->Bind();
        for (size_t i = 0; i < 6; i++)
            m_prefilterShader->SetMat4("u_ViewProjMatrices[" + std::to_string(i) + "]", captureProj * captureViews[i]);

        m_prefilterShader->SetInt("u_EnvMap", 1);
        glBindTextureUnit(1, renderCtx.GetTexture("u_EnvMap"));

        uint32_t maxMip = 5;
        for (size_t mip = 0; mip < maxMip; mip++)
        {
            unsigned int mipWidth  = 128 * std::pow(0.5, mip);
            unsigned int mipHeight = 128 * std::pow(0.5, mip);
            glViewport(0, 0, mipWidth, mipHeight);

            float roughness = (float)mip / (float)(maxMip - 1);
            m_prefilterShader->SetFloat("u_Roughness", roughness);

            glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_prefilteredMap->GetID(), mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            blrc::Renderer::DrawCube();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_hasExecuted = true;


        renderCtx.SetTexture("u_PrefilterMap", m_prefilteredMap->GetID());
    }

    void Shutdown() override
    {
        glDeleteFramebuffers(1, &m_fboID);
    }

private:
    blrc::Ref<blrc::Shader> m_prefilterShader;
    blrc::Ref<blrc::Cubemap> m_prefilteredMap;
    
    blrc::Ref<blrc::FrameBuffer> m_fbo;

    GLuint m_fboID{0};
    bool m_hasExecuted = false;
};