// ui/ui.cpp

#include "UI.hpp"
#include <bolero.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <implot.h>

#include "core/scene.hpp"
#include "core/render_context.hpp"


namespace blr::core
{


void UI::Init(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    m_initialized = true;
}

void UI::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::EndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

void UI::DrawPipelineStats(Scene& scene, const std::vector<Ref<RenderPass>>& passes)
{
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* version  = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    float totalCpuTimeMs    = 0.0f;
    float totalGpuTimeMs    = 0.0f;
    uint32_t totalDrawCalls = 0;

    for (const Ref<RenderPass>& pass : passes)
    {
        const PassStats& stats = pass->GetStats();
        totalCpuTimeMs  += stats.cpuTimeMs;
        totalGpuTimeMs  += stats.gpuTimeMs;
        totalDrawCalls  += stats.drawCalls;
    }

    if (ImGui::Begin("Pipeline Stats"))
    {
        ImGui::Text(renderer);
        ImGui::Text(version);
        ImGui::Spacing();
        ImGui::Text("TOTAL: CPU: %.2fms | GPU: %.2fms | Draws: %d",
            totalCpuTimeMs, totalGpuTimeMs, totalDrawCalls);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        for (const Ref<RenderPass>& pass : passes)
        {
            const PassStats& stats = pass->GetStats();
            ImGui::Text("[%s] CPU: %.2fms | GPU: %.2fms | Draws: %d",
                pass->GetName().c_str(), stats.cpuTimeMs, stats.gpuTimeMs, stats.drawCalls);
        }
    }
    ImGui::End();
}


} /* namespace blr::core */