#pragma once

#include <bolero.hpp>

namespace blrc = blr::core;


class PointShadowPass : public blrc::RenderPass
{
public:
    static constexpr int MAX_POINT_LIGHTS = 4;

    PointShadowPass(const blrc::Ref<blrc::Shader>& depthShader, uint32_t size = 512)
    : RenderPass("Point Shadow Pass")
    , m_depthShader(depthShader)
    , m_size(size)
    {
    }

    void Init() override
    {
        for (size_t i = 0; i < MAX_POINT_LIGHTS; i++)
            m_fbos.emplace_back(blrc::FrameBuffer::Create({ m_size, m_size, { {blrc::ImgFmt::Depth32F, true} } }));
    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        auto pointLights = scene.GetPointLights();
        if (pointLights.empty())
            return;

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        m_depthShader->Bind();

        int shadowMapIndex = 0;
        for (const auto& l : pointLights)
        {
            if (!l.castsShadow)
                continue;
            
            if (shadowMapIndex >= MAX_POINT_LIGHTS)
                break;


            m_fbos[shadowMapIndex]->Bind();

            glClear(GL_DEPTH_BUFFER_BIT);

            for (int i = 0; i < 6; i++)
                m_depthShader->SetMat4("u_ShadowMatrices[" + std::to_string(i) + "]", l.GetLightSpaceMatrices()[i]);

            m_depthShader->SetFloat("u_FarPlane", l.range);
            m_depthShader->SetVec3("u_LightPos", l.position);

            blrc::Renderer::DrawQueue(blrc::RenderQueueType::SHADOW_CASTER, m_depthShader.get()); 

            m_fbos[shadowMapIndex]->Unbind();


            // Pass to render context
            std::string texName = "u_PointDepthMapTex_" + std::to_string(shadowMapIndex);
            renderCtx.Set(texName, m_fbos[shadowMapIndex]->GetDepthAttachmentID());

            shadowMapIndex++;
        }

        glCullFace(GL_BACK);


        renderCtx.Set("u_NumPointShadows", shadowMapIndex);
    }

    void Shutdown() override
    {
        m_fbos.clear();
    }

private:
    std::vector<blrc::Ref<blrc::FrameBuffer>> m_fbos;
    blrc::Ref<blrc::Shader> m_depthShader;

    uint32_t m_size;
};