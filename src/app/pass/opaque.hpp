// app/pass/opaque.hpp

#pragma once

#include "renderer/pass.hpp"
#include "renderer/renderer.hpp"


class OpaquePass : public blr::core::RenderPass
{
public:
    OpaquePass() : RenderPass("Main Opaque Pass") {}

    void Init() override
    {
        // Pass Specific GL states
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    void Execute(blr::core::Scene& scene) override
    {
        // Setup render target
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // Clear screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw with using this RenderPass 'instructions'
        // nullptr means use the material's shaders
        blr::core::Renderer::DrawQueue(nullptr); 
    }

    void Shutdown() override {}
};