// ui/ui.h

#pragma once

#include "utils/base.hpp"

#include <vector>
#include <string>

struct GLFWwindow;


namespace blr::core
{


class Scene;
class RenderPass;
class RenderContext;

class UI 
{
public:
    void Init(GLFWwindow* window);
    void BeginFrame();
    void EndFrame();
    void Shutdown();

    void DrawPipelineStats(Scene& scene, const std::vector<Ref<RenderPass>>& passes);

    void DrawProperties(Scene& scene, RenderContext& renderCtx);

    void DrawScene(RenderContext& renderCtx);

    void DrawExport(Scene& scene, RenderContext& renderCtx);

    void CaptureViewport(RenderContext& renderCtx, const std::string& path);

private:
    bool m_initialized = false;
};


} /* namespace blr::core */
