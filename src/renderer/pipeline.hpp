// renderer/pipeline.hpp

#pragma once

#include "renderer/pass.hpp"
#include "core/scene.hpp"
#include "renderer/renderer.hpp"

#include <vector>


namespace blr::core
{


class RenderPipeline
{
public:
    RenderPipeline() = default;
    
    ~RenderPipeline() 
    { 
        Shutdown(); 
    }

    void AddPass(Ref<RenderPass> pass)
    {
        pass->Init();
        m_passes.push_back(pass);
    }

    void Execute(Scene& scene)
    {
        // Submits the scene data (renderables, lights, transforms) to the GPU
        scene.SubmitToRenderer();

        // Uploads instance matrices and light data SSBOs to the shader
        Renderer::UploadBuffers();

        // Iterates on every (user defined) render pass
        for (auto& pass : m_passes)
            pass->Execute(scene);
    }

    void Shutdown()
    {
        for (auto& pass : m_passes)
            pass->Shutdown();

        m_passes.clear();
    }

private:
    std::vector<Ref<RenderPass>> m_passes;
};


} /* namespace blr::core */