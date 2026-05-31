// renderer/pass.hpp

#pragma once

#include "core/types.hpp"
#include "core/render_context.hpp"
#include <string>
#include "utils/gpu_timer.hpp"


namespace blr::core
{


struct PassStats
{
    float    cpuTimeMs = 0.0f;
    float    gpuTimeMs = 0.0f;
    uint32_t drawCalls = 0;
};

class Scene;

class RenderPass
{
public:
    RenderPass(const std::string& name) : m_name(name) {}
    virtual ~RenderPass() = default;

    // Called once when added to the pipeline by RenderPipeline::AddPass()
    virtual void Init() = 0;

    // This is where you configure GL state and issue Renderer draw commands.
    // Called every frame.
    virtual void Execute(Scene& scene, RenderContext& renderCtx) = 0;

    // Resizing behavior.
    // Useful for enabling window responsiveness
    virtual void OnResize(uint32_t width, uint32_t height)
    {
    }

    // Called when done
    virtual void Shutdown() = 0;

    const std::string& GetName() const { return m_name; }

    utils::GpuTimer& GetGpuTimer() { return m_gpuTimer; }
    PassStats&       GetStats()    { return m_stats; }
    const PassStats& GetStats() const { return m_stats; }

protected:
    std::string m_name;

private:
    utils::GpuTimer m_gpuTimer;
    PassStats       m_stats;
};


} /* namespace blr::core */