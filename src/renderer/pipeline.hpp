// renderer/pipeline.hpp

#pragma once

#include "renderer/pass.hpp"
#include "core/scene.hpp"
#include "renderer/renderer.hpp"
#include "utils/cpu_timer.hpp"

#include <vector>


namespace blr::core
{


class RenderContext;

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

    void Execute(Scene& scene, RenderContext& renderCtx)
    {
        // Submits the scene data (renderables, lights, transforms) to the GPU
        scene.SubmitToRenderer();

        // Uploads instance matrices and light data SSBOs to the shader
        Renderer::UploadBuffers();

        // Iterates on every (user defined) render pass
        for (auto& pass : m_passes)
        {
            pass->GetGpuTimer().Start();
            uint32_t drawCallsBefore = Renderer::GetRenderStats().drawCalls;
            float cpuTime = 0.0f;
            
            {
                utils::CpuTimer cpuTimer(cpuTime);
                pass->Execute(scene, renderCtx);
            }
            
            pass->GetGpuTimer().Stop();
            
            // Profiling
            PassStats& stats = pass->GetStats();
            stats.name       = pass->GetName();
            stats.cpuTimeMs  = cpuTime;
            stats.gpuTimeMs  = pass->GetGpuTimer().GetElapsedMs();
            stats.drawCalls  = Renderer::GetRenderStats().drawCalls - drawCallsBefore;
        }
    }

    void OnResize(uint32_t width, uint32_t height)
    {
        for (auto& pass : m_passes)
            pass->OnResize(width, height);
    }

    void Shutdown()
    {
        for (auto& pass : m_passes)
            pass->Shutdown();

        m_passes.clear();
    }

    const std::vector<Ref<RenderPass>>& GetPasses() const { return m_passes; }

private:
    std::vector<Ref<RenderPass>> m_passes;
};


} /* namespace blr::core */