#pragma once

#include <bolero.hpp>


namespace blrc = blr::core;

class DirShadowPass : public blrc::RenderPass
{
public:
    static constexpr int MAX_DIR_LIGHTS = 4;

    DirShadowPass(const blrc::Ref<blrc::Shader>& depthShader)
    : RenderPass("Directional Shadow Pass")
    , m_depthShader(depthShader)
    {
    }

    void Init() override
    {
        for (size_t i = 0; i < MAX_DIR_LIGHTS; i++)
            m_fbos.emplace_back(blrc::FrameBuffer::Create({ 512, 512, { blrc::ImgFmt::Depth32F } }));
    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        auto dirLights = scene.GetDirLights();
        if (dirLights.empty())
            return;

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        m_depthShader->Bind();

        blrc::vec3 targetPos = blrc::vec3(0.0f); 

        int shadowMapIndex = 0;
        for (const auto& l : dirLights)
        {
            if (!l.castsShadow)
                continue;
            
            if (shadowMapIndex >= MAX_DIR_LIGHTS)
                break;

            m_fbos[shadowMapIndex]->Bind();
            
            glClear(GL_DEPTH_BUFFER_BIT);

            blrc::mat4 lightSpaceMat = l.GetLightSpaceMat(targetPos);
            m_depthShader->SetMat4("u_LightSpaceMat", lightSpaceMat);

            blrc::Renderer::DrawQueue(blrc::RenderQueueType::SHADOW_CASTER, m_depthShader.get()); 

            m_fbos[shadowMapIndex]->Unbind();


            // Pass to render context
            std::string texName = "u_DirDepthMapTex_" + std::to_string(shadowMapIndex);
            std::string matName = "u_DirLightSpaceMat_" + std::to_string(shadowMapIndex);
            renderCtx.Set(texName, m_fbos[shadowMapIndex]->GetDepthAttachmentID());
            renderCtx.Set(matName, lightSpaceMat);

            shadowMapIndex++;
        }

        glCullFace(GL_BACK);


        renderCtx.Set("u_NumDirShadows", shadowMapIndex);
    }

    void Shutdown() override
    {
        m_fbos.clear();
    }

private:
    std::vector<blrc::Ref<blrc::FrameBuffer>> m_fbos;
    blrc::Ref<blrc::Shader> m_depthShader;
};