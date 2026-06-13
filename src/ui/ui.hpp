// ui/ui.h

#pragma once

#include <bolero.hpp>

struct GLFWwindow;


namespace blr::core
{


class Scene;
class RenderPass;

class UI 
{
public:
    void Init(GLFWwindow* window);
    void BeginFrame();
    void EndFrame();
    void Shutdown();

    void DrawPipelineStats(Scene& scene, const std::vector<Ref<RenderPass>>& passes);

    void DrawProperties(Scene& scene, RenderContext& renderCtx);

private:
    bool m_initialized = false;
};


} /* namespace blr::core */
