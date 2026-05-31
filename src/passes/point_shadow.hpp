#pragma once

#include <bolero.hpp>
#include "core/lights.hpp"

namespace blrc = blr::core;


class PointShadowPass : public blrc::RenderPass
{
public:
    PointShadowPass(const blrc::Ref<blrc::Shader>& depthShader)
    : RenderPass("Point Shadow Pass")
    , m_depthShader(depthShader)
    {
    }

    void Init() override
    {
        m_fbo = blrc::FrameBuffer::Create({ 1024, 1024, { {blrc::ImgFmt::Depth32F, true} } });
    }

    void Execute(blrc::Scene& scene) override
    {
        m_fbo->Bind();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glClear(GL_DEPTH_BUFFER_BIT);

        auto pointLights = scene.GetPointLights();
        if (pointLights.empty()) 
        {
            m_fbo->Unbind();
            return;
        }

        blrc::PointLight point = pointLights[0];
        blrc::vec3 lightPos = point.position;

        float aspect = 1.0f;
        float near   = 1.0f;
        float far    = point.range;
        blrc::mat4 shadowProj = blrc::Perspective(blrc::DegToRad(90.0f), aspect, near, far);

        std::vector<blrc::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * blrc::LookAt(lightPos, lightPos + blrc::vec3( 1, 0, 0), blrc::vec3(0,-1, 0))); // +X
        shadowTransforms.push_back(shadowProj * blrc::LookAt(lightPos, lightPos + blrc::vec3(-1, 0, 0), blrc::vec3(0,-1, 0))); // -X
        shadowTransforms.push_back(shadowProj * blrc::LookAt(lightPos, lightPos + blrc::vec3( 0, 1, 0), blrc::vec3(0, 0, 1))); // +Y
        shadowTransforms.push_back(shadowProj * blrc::LookAt(lightPos, lightPos + blrc::vec3( 0,-1, 0), blrc::vec3(0, 0,-1))); // -Y
        shadowTransforms.push_back(shadowProj * blrc::LookAt(lightPos, lightPos + blrc::vec3( 0, 0, 1), blrc::vec3(0,-1, 0))); // +Z
        shadowTransforms.push_back(shadowProj * blrc::LookAt(lightPos, lightPos + blrc::vec3( 0, 0,-1), blrc::vec3(0,-1, 0))); // -Z

        m_depthShader->Bind();
        for (int i = 0; i < 6; ++i)
        {
            m_depthShader->SetMat4("u_ShadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        }
        m_depthShader->SetFloat("u_FarPlane", far);
        m_depthShader->SetVec3("u_LightPos", lightPos);

        blrc::Renderer::DrawQueue(m_depthShader.get()); 

        glCullFace(GL_BACK);
        m_fbo->Unbind();
    }

    void Shutdown() override {}

    GLuint GetDepthMap() const { return m_fbo->GetDepthAttachmentID(); }

private:
    blrc::Ref<blrc::FrameBuffer> m_fbo;
    blrc::Ref<blrc::Shader> m_depthShader;
};