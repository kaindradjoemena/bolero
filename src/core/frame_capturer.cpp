// core/frame_capturer.cpp

#include "core/frame_capturer.hpp"

#include "renderer/renderer.hpp"
#include "core/camera.hpp"
#include <glad/glad.h>
#include <stdexcept>
#include <cmath>


namespace blr::core
{


ImageBuffer FrameCapturer::CapturePipeline(RenderPipeline& pipeline, Scene& scene, RenderContext& context, 
    uint32_t width, uint32_t height, FitMode fitMode, const std::string& targetTextureName)
{
    if (width == 0 || height == 0)
        throw std::runtime_error("Capture dimensions cannot be zero.");

    uint32_t currentW = Renderer::GetViewportWidth();
    uint32_t currentH = Renderer::GetViewportHeight();

    Camera* cam = scene.GetCam();
    float prevAspect = 1.0f;
    float prevFov = 1.0f;
    if (cam)
    {
        prevAspect = cam->GetAspect();
        prevFov = cam->GetFov();

        float targetAspect = static_cast<float>(width) / static_cast<float>(height);
        float currentAspect = static_cast<float>(currentW) / static_cast<float>(currentH);

        if (fitMode == FitMode::H)
        {
            float newFov = 2.0f * atan(tan(prevFov / 2.0f) * (currentAspect / targetAspect));
            cam->SetFov(newFov);
        }
        cam->SetAspect(targetAspect);
    }

    Renderer::SetViewportResolution(width, height);
    context.ClearTransient();
    pipeline.Execute(scene, context);

    GLuint targetTex = context.Get<GLuint>(targetTextureName, 0);
    if (targetTex == 0)
    {
        Renderer::SetViewportResolution(currentW, currentH);
        throw std::runtime_error("Capture failed: Texture '" + targetTextureName + "' not found in context.");
    }

    ImageBuffer buffer;
    buffer.width = width;
    buffer.height = height;
    buffer.channels = 4;
    buffer.pixels.resize(width * height * 4);

    glBindTexture(GL_TEXTURE_2D, targetTex);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    Renderer::SetViewportResolution(currentW, currentH);

    if (cam)
    {
        cam->SetAspect(prevAspect);
        cam->SetFov(prevFov);
    }

    return buffer;
}


} /* namespace blr::core */
