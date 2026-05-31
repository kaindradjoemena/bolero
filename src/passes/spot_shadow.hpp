#pragma once

#include <bolero.hpp>
#include "core/lights.hpp"

namespace blrc = blr::core;


class SpotShadowPass : public blrc::RenderPass
{
public:
    SpotShadowPass(const blrc::Ref<blrc::Shader>& depthShader)
    : RenderPass("Spot Shadow Pass")
    , m_depthShader(depthShader)
    {
    }

    void Init() override
    {
        m_fbo = blrc::FrameBuffer::Create({ 1024, 1024, { blrc::ImgFmt::Depth32F } });
    }

    void Execute(blrc::Scene& scene) override
    {
        m_fbo->Bind();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glClear(GL_DEPTH_BUFFER_BIT);

        auto spotLights = scene.GetSpotLights();
        if (spotLights.empty()) 
        {
            m_fbo->Unbind();
            return;
        }

        blrc::SpotLight spot = spotLights[0];

        blrc::vec3 lightPos = spot.position;
        blrc::vec3 lightDir = blrc::Norm(spot.direction);

        blrc::vec3 up = (abs(lightDir.y) > 0.999f) ? blrc::vec3(0.0f, 0.0f, 1.0f) : blrc::vec3(0.0f, 1.0f, 0.0f);

        blrc::mat4 lightView = blrc::LookAt(lightPos, lightPos + lightDir, up);
        
        float fov = acos(spot.outerCos) * 2.0f;
        blrc::mat4 lightProj = blrc::Perspective(fov, 1.0f, 1.0f, 100.0f);
        
        m_lightSpaceMatrix = lightProj * lightView;

        blrc::Renderer::UpdateCameraUBO(lightView, lightProj, lightPos);
        blrc::Renderer::DrawQueue(m_depthShader.get()); 

        glCullFace(GL_BACK);
        m_fbo->Unbind();
    }

    void Shutdown() override {}

    GLuint GetDepthMap() const { return m_fbo->GetDepthAttachmentID(); }
    const blrc::mat4& GetLightSpaceMat() const { return m_lightSpaceMatrix; }

private:
    blrc::Ref<blrc::FrameBuffer> m_fbo;
    blrc::mat4 m_lightSpaceMatrix;
    blrc::Ref<blrc::Shader> m_depthShader;
};