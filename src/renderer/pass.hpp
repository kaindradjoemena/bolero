// renderer/pass.hpp

#pragma once

#include "core/render_context.hpp"
#include "utils/gpu_timer.hpp"

#include <string>
#include <functional>
#include <vector>


namespace blr::core
{


struct PassStats
{
    std::string name;
    float    cpuTimeMs = 0.0f;
    float    gpuTimeMs = 0.0f;
    uint32_t drawCalls = 0;
};

class Scene;

class RenderPass
{
public:
    using InitFunc = std::function<void(RenderPass&)>;
    using ExecFunc = std::function<void(Scene&, RenderContext&, RenderPass&)>;
    using Action   = std::function<void()>;

    RenderPass(const std::string& name, InitFunc initFunc, ExecFunc execFunc)
    : m_name(name)
    , m_initFunc(initFunc)
    , m_execFunc(execFunc)
    {
    }
    
    virtual ~RenderPass() = default;

    void Init()
    {
        if (m_initFunc)
            m_initFunc(*this);
    }

    void Execute(Scene& scene, RenderContext& renderCtx)
    {
        if (m_execFunc)
            m_execFunc(scene, renderCtx, *this);
    }

    void OnWindowResize(uint32_t width, uint32_t height)
    {
    }

    void Shutdown()
    {
    }


    void DispatchAction(Action action)
    {
        m_actionQueue.push_back(action);
    }
    
    std::vector<Action>& GetActions()
    {
        return m_actionQueue;
    }
    
    void ClearActions()
    {
        m_actionQueue.clear();
    }


    const std::string& GetName() const { return m_name; }

    utils::GpuTimer& GetGpuTimer() { return m_gpuTimer; }
    PassStats&       GetStats()    { return m_stats; }
    const PassStats& GetStats() const { return m_stats; }

protected:
    std::string m_name;

private:
    InitFunc m_initFunc;
    ExecFunc m_execFunc;

    std::vector<Action> m_actionQueue;

    utils::GpuTimer m_gpuTimer;
    PassStats       m_stats;
};


} /* namespace blr::core */