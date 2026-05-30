#pragma once

#include "core/types.hpp"
#include "renderer/pass.hpp"
#include "renderer/renderer.hpp"

#include "core/wrappers/framebuffer.hpp"
#include "core/shader.hpp"
#include "core/lights.hpp"

namespace blrc = blr::core;


class ShadowPass : public blrc::RenderPass
{
public:
    ShadowPass(const blrc::Ref<blrc::Shader>& depthShader)
    : RenderPass("Shadow Pass")
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

        auto dirLights = scene.GetDirLights();
        if (dirLights.empty()) 
            return;
            
        blrc::DirLight sun = dirLights[0];

        blrc::vec3 lightDir  = blrc::Norm(sun.direction);
        blrc::vec3 targetPos = blrc::vec3(0.0f, 0.0f, 0.0f);
        float shadowDist     = 40.0f;
        blrc::vec3 lightPos  = targetPos - (lightDir * shadowDist);

        blrc::mat4 lightView = blrc::LookAt(lightPos, targetPos, blrc::vec3(0.0f, 1.0f, 0.0f));
        blrc::mat4 lightProj = blrc::Ortho(-40.0f, 40.0f, -40.0f, 40.0f, 1.0f, 80.0f);
        m_lightSpaceMatrix   = lightProj * lightView;

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