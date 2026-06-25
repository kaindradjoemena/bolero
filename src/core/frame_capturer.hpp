// core/frame_capturer.hpp

#pragma once

#include "core/scene.hpp"
#include "renderer/pipeline.hpp"
#include "core/render_context.hpp"
#include "core/image_buffer.hpp"


namespace blr::core
{


enum class FitMode { V, H };


class FrameCapturer
{
public:
    static ImageBuffer CapturePipeline(RenderPipeline& pipeline, Scene& scene, RenderContext& context, 
        uint32_t width, uint32_t height, FitMode fitMode = FitMode::V, const std::string& targetTextureName = "POST_PASS_TEX");
};


} /* namespace blr::core */
