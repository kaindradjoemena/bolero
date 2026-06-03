// passes/ui.hpp

#pragma once

#include <bolero.hpp>
#include "core/render_context.hpp"
#include "ui/ui.hpp"

namespace blrc = blr::core;

class UIPass : public blrc::RenderPass
{
public:
    UIPass(GLFWwindow* window, const std::vector<blrc::Ref<blrc::RenderPass>>& passes)
    : RenderPass("UI Pass")
    , m_window(window)
    , m_passes(passes)
    {
    }

    void Init() override
    {
        m_ui.Init(m_window);
    }

    void Execute(blrc::Scene& scene, blrc::RenderContext& renderCtx) override
    {
        m_ui.BeginFrame();


        m_ui.DrawPipelineStats(scene, m_passes);


        m_ui.EndFrame();
    }

    virtual void OnResize(uint32_t width, uint32_t height) override
    {
    }

    void Shutdown() override
    {
        m_ui.Shutdown();
    }

private:
    blrc::UI m_ui;

    GLFWwindow* m_window;
    const std::vector<blrc::Ref<RenderPass>>& m_passes;
};