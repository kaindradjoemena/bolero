#pragma once

#include <bolero.hpp>

namespace blrc = blr::core;

class SpotShadowPass : public blrc::RenderPass
{
public:
    static constexpr int MAX_SPOT_LIGHTS = 4;

    SpotShadowPass(const blrc::Ref<blrc::Shader>& depthShader)
    : RenderPass("Spot Shadow Pass")
    , m_depthShader(depthShader)
    {
    }

    void Init() override
    {
        for (size_t i = 0; i < MAX_SPOT_LIGHTS; i++)
            m_fbos.emplace_back(blrc::FrameBuffer::Create({ 512, 512, { blrc::ImgFmt::Depth32F } }));
    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        auto spotLights = scene.GetSpotLights();
        if (spotLights.empty()) 
            return;

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        m_depthShader->Bind();

        int shadowMapIndex = 0;
        for (const auto& l : spotLights)
        {
            if (!l.castsShadow)
                continue;
            
            if (shadowMapIndex >= MAX_SPOT_LIGHTS)
                break;

            m_fbos[shadowMapIndex]->Bind();
            
            glClear(GL_DEPTH_BUFFER_BIT);

            blrc::mat4 lightSpaceMat = l.GetLightSpaceMat();

            m_depthShader->SetMat4("u_LightSpaceMat", lightSpaceMat);

            blrc::Renderer::DrawQueue(blrc::RenderQueueType::SHADOW_CASTER, m_depthShader.get()); 

            m_fbos[shadowMapIndex]->Unbind();


            // Pass to render context
            std::string texName = "u_SpotDepthMapTex_" + std::to_string(shadowMapIndex);
            std::string matName = "u_SpotLightSpaceMat_" + std::to_string(shadowMapIndex);
            renderCtx.SetTexture(texName, m_fbos[shadowMapIndex]->GetDepthAttachmentID());
            renderCtx.SetMat4(matName, lightSpaceMat);

            shadowMapIndex++;
        }

        glCullFace(GL_BACK);


        renderCtx.SetInt("u_NumSpotShadows", shadowMapIndex);
    }

    void Shutdown() override
    {
        m_fbos.clear();
    }

private:
    std::vector<blrc::Ref<blrc::FrameBuffer>> m_fbos;
    blrc::Ref<blrc::Shader> m_depthShader;
};