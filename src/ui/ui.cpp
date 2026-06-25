// ui/ui.cpp

#include "UI.hpp"

#include <algorithm>

#include <bolero.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stb_image_write.h>
#include <implot.h>

#include "core/scene.hpp"
#include "core/render_context.hpp"


namespace blr::core
{


void UI::Init(GLFWwindow* window)
{
    m_initialized = true;
}

void UI::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Docking
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
}

void UI::EndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::Shutdown()
{
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
        ImGui::Text("TOTAL: CPU: %.2fms | GPU: %.2fms | Draws: %d", totalCpuTimeMs, totalGpuTimeMs, totalDrawCalls);
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

void UI::DrawProperties(Scene& scene, RenderContext& renderCtx)
{
    if (!ImGui::Begin("Properties"))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("PropertyTabs"))
    {
        // ===== CAMERA =====
        if (ImGui::BeginTabItem("Camera"))
        {
            Camera* cam = scene.GetCam();
            if (cam)
            {
                ImGui::SeparatorText("Projection");
                {
                    float fovDeg = glm::degrees(cam->GetFov());
                    if (ImGui::SliderFloat("FOV", &fovDeg,
                        glm::degrees(Camera::MIN_FOV),
                        glm::degrees(Camera::MAX_FOV), "%.1f deg"))
                        cam->SetFov(glm::radians(fovDeg));

                    float nearVal = cam->GetNear();
                    if (ImGui::SliderFloat("Near", &nearVal,
                        Camera::MIN_NEAR, cam->GetFar(), "%.4f", ImGuiSliderFlags_Logarithmic))
                        cam->SetNear(nearVal);

                    float farVal = cam->GetFar();
                    if (ImGui::SliderFloat("Far", &farVal,
                        cam->GetNear(), Camera::MAX_FAR, "%.1f", ImGuiSliderFlags_Logarithmic))
                        cam->SetFar(farVal);
                }

                ImGui::Spacing();
                ImGui::SeparatorText("Orbit");
                {
                    float yawDeg = glm::degrees(cam->GetYaw());
                    if (ImGui::SliderFloat("Yaw", &yawDeg, -180.0f, 180.0f, "%.1f deg"))
                        cam->SetYaw(glm::radians(yawDeg));

                    float pitchDeg = glm::degrees(cam->GetPitch());
                    if (ImGui::SliderFloat("Pitch", &pitchDeg,
                        glm::degrees(Camera::MIN_PITCH),
                        glm::degrees(Camera::MAX_PITCH), "%.1f deg"))
                        cam->SetPitch(glm::radians(pitchDeg));

                    float dist = cam->GetDistance();
                    if (ImGui::SliderFloat("Distance", &dist,
                        Camera::MIN_DISTANCE, Camera::MAX_DISTANCE, "%.2f", ImGuiSliderFlags_Logarithmic))
                        cam->SetDistance(dist);

                    vec3 target = cam->GetTarget();
                    if (ImGui::DragFloat3("Target", &target.x, 0.1f))
                        cam->SetTarget(target);
                }

                ImGui::Spacing();
                ImGui::SeparatorText("Sensitivity");
                {
                    float rot = cam->GetRotSense();
                    if (ImGui::DragFloat("Rotation", &rot, 0.0001f, 0.0f, 1.0f, "%.4f"))
                        cam->SetRotSense(rot);

                    float pan = cam->GetPanSense();
                    if (ImGui::DragFloat("Pan", &pan, 0.0001f, 0.0f, 1.0f, "%.4f"))
                        cam->SetPanSense(pan);

                    float scr = cam->GetScrSense();
                    if (ImGui::DragFloat("Scroll", &scr, 0.001f, 0.0f, 1.0f, "%.4f"))
                        cam->SetScrSense(scr);
                }

                ImGui::Spacing();
                if (ImGui::Button("Reset Camera"))
                {
                    cam->SetFov(Camera::DEFAULT_FOV);
                    cam->SetNear(Camera::DEFAULT_NEAR);
                    cam->SetFar(Camera::DEFAULT_FAR);
                    cam->SetYaw(Camera::DEFAULT_YAW);
                    cam->SetPitch(Camera::DEFAULT_PITCH);
                    cam->SetDistance(Camera::DEFAULT_DISTANCE);
                    cam->SetTarget(Camera::DEFAULT_TARGET);
                    cam->SetRotSense(Camera::DEFAULT_ROT_SENS);
                    cam->SetPanSense(Camera::DEFAULT_PAN_SENS);
                    cam->SetScrSense(Camera::DEFAULT_SCR_SENS);
                }
            }
            else
            {
                ImGui::TextDisabled("No camera attached to scene.");
            }

            ImGui::EndTabItem();
        }

        // ===== LIGHTS =====
        if (ImGui::BeginTabItem("Lights"))
        {
            // --- Directional ---
            auto& dirLights = scene.GetDirLights();
            if (!dirLights.empty())
            {
                ImGui::SeparatorText("Directional");
                for (size_t i = 0; i < dirLights.size(); i++)
                {
                    ImGui::PushID(static_cast<int>(i));
                    char header[32];
                    snprintf(header, sizeof(header), "Dir Light %zu", i);
                    if (ImGui::TreeNode(header))
                    {
                        DirLight& light = dirLights[i];
                        ImGui::ColorEdit3("Color",       &light.base.color.x);
                        ImGui::DragFloat("Power",        &light.base.power,    0.1f,  0.0f, 1000.0f);
                        ImGui::DragFloat3("Direction",   &light.direction.x,   0.01f, -1.0f, 1.0f);
                        ImGui::Checkbox("Casts Shadow",  &light.castsShadow);
                        if (light.castsShadow)
                        {
                            ImGui::DragFloat("Shadow Size", &light.shadowSize, 0.5f,  1.0f,  500.0f);
                            ImGui::DragFloat("Shadow Near", &light.shadowNear, 0.01f, 0.001f, light.shadowFar);
                            ImGui::DragFloat("Shadow Far",  &light.shadowFar,  0.5f,  light.shadowNear, 500.0f);
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
            }

            // --- Point ---
            auto& pointLights = scene.GetPointLights();
            if (!pointLights.empty())
            {
                ImGui::SeparatorText("Point");
                for (size_t i = 0; i < pointLights.size(); i++)
                {
                    ImGui::PushID(static_cast<int>(1000 + i));
                    char header[32];
                    snprintf(header, sizeof(header), "Point Light %zu", i);
                    if (ImGui::TreeNode(header))
                    {
                        PointLight& light = pointLights[i];
                        ImGui::ColorEdit3("Color",      &light.base.color.x);
                        ImGui::DragFloat("Power",       &light.base.power,  0.1f, 0.0f, 1000.0f);
                        ImGui::DragFloat3("Position",   &light.position.x,  0.1f);
                        ImGui::DragFloat("Range",       &light.range,       0.1f, 0.0f, 1000.0f);
                        ImGui::Checkbox("Casts Shadow", &light.castsShadow);
                        if (light.castsShadow)
                            ImGui::DragFloat("Shadow Near", &light.shadowNear, 0.001f, 0.001f, light.range);
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
            }

            // --- Spot ---
            auto& spotLights = scene.GetSpotLights();
            if (!spotLights.empty())
            {
                ImGui::SeparatorText("Spot");
                for (size_t i = 0; i < spotLights.size(); i++)
                {
                    ImGui::PushID(static_cast<int>(2000 + i));
                    char header[32];
                    snprintf(header, sizeof(header), "Spot Light %zu", i);
                    if (ImGui::TreeNode(header))
                    {
                        SpotLight& light = spotLights[i];
                        ImGui::ColorEdit3("Color",      &light.base.color.x);
                        ImGui::DragFloat("Power",       &light.base.power,    0.1f,  0.0f, 1000.0f);
                        ImGui::DragFloat3("Position",   &light.position.x,    0.1f);
                        ImGui::DragFloat3("Direction",  &light.direction.x,   0.01f, -1.0f, 1.0f);
                        ImGui::DragFloat("Length",      &light.length,        0.1f,  0.0f,  1000.0f);

                        float innerDeg = glm::degrees(acos(glm::clamp(light.innerCos, -1.0f, 1.0f)));
                        float outerDeg = glm::degrees(acos(glm::clamp(light.outerCos, -1.0f, 1.0f)));

                        bool changed = false;
                        changed |= ImGui::DragFloat("Inner Angle", &innerDeg, 0.1f, 0.0f, outerDeg, "%.1f deg");
                        changed |= ImGui::DragFloat("Outer Angle", &outerDeg, 0.1f, innerDeg, 89.0f, "%.1f deg");
                        if (changed)
                        {
                            light.innerCos = cos(glm::radians(innerDeg));
                            light.outerCos = cos(glm::radians(outerDeg));
                        }

                        ImGui::Checkbox("Casts Shadow",  &light.castsShadow);
                        if (light.castsShadow)
                            ImGui::DragFloat("Shadow Near", &light.shadowNear, 0.001f, 0.001f, light.length);
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
            }

            if (dirLights.empty() && pointLights.empty() && spotLights.empty())
                ImGui::TextDisabled("No lights in scene.");

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Rendering"))
        {
            ImGui::SeparatorText("Background Color");
            {
                glm::vec4 backgroundCol = renderCtx.Get<glm::vec4>("BACKGROUND_COLOR", glm::vec4(0.0f, 0.0f, 0.0f, 1.0));
                if (ImGui::ColorEdit4("Color", &backgroundCol.x))
                    renderCtx.Set("BACKGROUND_COLOR", backgroundCol, Lifetime::PERSISTENT);

            }

            ImGui::SeparatorText("Post Processing");
            {
                float exposure = renderCtx.Get<float>("u_Exposure", 1.0f);
                if (ImGui::DragFloat("Exposure", &exposure, 0.01f, 0.0f, 10.0f, "%.2f EV"))
                    renderCtx.Set("u_Exposure", exposure, Lifetime::PERSISTENT);
            }

            ImGui::Spacing();
            ImGui::SeparatorText("Environment");
            {
                float envPow = renderCtx.Get<float>("u_EnvironmentPower", 1.0f);
                if (ImGui::SliderFloat("Skybox Power", &envPow, 0.0f, 10.0f, "%.2f"))
                    renderCtx.Set("u_EnvironmentPower", envPow, Lifetime::PERSISTENT);

                
                float envBlur = renderCtx.Get<float>("u_EnvironmentBlur", 0.9f);
                if (ImGui::SliderFloat("Skybox Blur", &envBlur, 0.0f, 10.0f, "%.2f"))
                    renderCtx.Set("u_EnvironmentBlur", envBlur, Lifetime::PERSISTENT);
                
                float envRotRad = renderCtx.Get<float>("u_EnvironmentRot", 0.0f);
                float envRotDeg = glm::degrees(envRotRad);
                if (ImGui::SliderFloat("Skybox Rotation", &envRotDeg, 0.0f, 360.0f, "%.2f"))
                    renderCtx.Set("u_EnvironmentRot", glm::radians(envRotDeg), Lifetime::PERSISTENT);
            }

            ImGui::SeparatorText("Anti-Aliasing");
            {
                // SSAA Scale
                uint32_t scale = Renderer::GetRenderScale();
                int scaleInt = static_cast<int>(scale);
                if (ImGui::SliderInt("SSAA Scale", &scaleInt, 1, 4))
                    Renderer::SetRenderScale(scaleInt);
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void UI::DrawScene(RenderContext& renderCtx)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    if (ImGui::Begin("Scene Viewport"))
    {
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();

        uint32_t vpW = std::max((uint32_t)viewportSize.x, 1u);
        uint32_t vpH = std::max((uint32_t)viewportSize.y, 1u);

        Renderer::SetViewportResolution(vpW, vpH);

        GLuint finalTex = renderCtx.Get<GLuint>("POST_PASS_TEX", 0);
        if (finalTex != 0)
            ImGui::Image((void*)(intptr_t)finalTex, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();

    ImGui::PopStyleVar();
}

void UI::DrawExport(Scene& scene, RenderContext& renderCtx)
{
    if (!ImGui::Begin("Export"))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("Export Tabs"))
    {
        // ===== VIEWPORT CAPTURE =====
        if (ImGui::BeginTabItem("Viewport"))
        {
            static char exportPath[256] = "capture.png";

            static std::string statusMsg = "";
            static ImVec4 statusColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

            if (ImGui::InputText("Save Path", exportPath, IM_ARRAYSIZE(exportPath)))
                statusMsg = ""; 

            std::string pathStr = exportPath;
            bool isPathEmpty = pathStr.empty();

            std::string ext = std::filesystem::path(pathStr).extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });

            bool isValidExt = (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga");

            bool isDirValid = true;
            try
            {
                std::filesystem::path p(pathStr);
                std::filesystem::path dir = p.parent_path();
                if (!dir.empty() && !std::filesystem::exists(dir))
                    isDirValid = false;
            }
            catch (const std::exception&)
            {
                isDirValid = false; 
            }

            bool isValid = !isPathEmpty && isValidExt && isDirValid;

            if (!isValid)
                ImGui::BeginDisabled();

            if (ImGui::Button("Capture"))
            {
                try
                {
                    CaptureViewport(renderCtx, pathStr);
                    statusMsg = "Saved to " + pathStr;
                    statusColor = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
                } 
                catch (const std::exception& e)
                {
                    statusMsg = std::string("Error: ") + e.what();
                    statusColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
                }
            }

            if (!isValid)
            {
                ImGui::EndDisabled();
                ImGui::SameLine();
                if (isPathEmpty)
                    ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Path cannot be empty!");
                else if (!isValidExt)
                    ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Use .png, .jpg, .bmp, or .tga");
                else if (!isDirValid)
                    ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Directory does not exist!");
            }

            if (!statusMsg.empty())
            {
                ImGui::Spacing();
                ImGui::TextColored(statusColor, "%s", statusMsg.c_str());
            }

            ImGui::EndTabItem();
        }

        // ===== OFFLINE RENDERING =====
        if (ImGui::BeginTabItem("Offline"))
        {
            static int targetSize[2] = {1920, 1080};
            ImGui::DragInt2("Resolution", targetSize, 1.0f, 64, 8192);

            static int currentFitMode = 0;
            const char* fitModes[] = { "Vertical (Standard Crop)", "Horizontal (Lock Width)" };
            ImGui::Combo("Camera Fit Mode", &currentFitMode, fitModes, IM_ARRAYSIZE(fitModes));

            ImGui::Spacing();

            static bool overrideSettings = false;
            ImGui::Checkbox("Override Live Rendering Settings", &overrideSettings);
            
            static float exp = 1.0f, envPow = 1.0f, envBlr = 0.9f, envRt = 0.0f;
            if (overrideSettings)
            {
                ImGui::Indent();
                ImGui::DragFloat("Exposure", &exp, 0.01f, 0.0f, 10.0f, "%.2f EV");
                ImGui::SliderFloat("Skybox Power", &envPow, 0.0f, 10.0f, "%.2f");
                ImGui::SliderFloat("Skybox Blur", &envBlr, 0.0f, 10.0f, "%.2f");
                float envRtDeg = glm::degrees(envRt);
                if (ImGui::SliderFloat("Skybox Rotation", &envRtDeg, 0.0f, 360.0f, "%.2f"))
                    envRt = glm::radians(envRtDeg);
                ImGui::Unindent();
            }
            ImGui::Spacing();

            static char exportPath[256] = "offline_render.png";
            ImGui::InputText("Save Path", exportPath, IM_ARRAYSIZE(exportPath));

            static std::string statusMsg = "";
            static ImVec4 statusColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

            // Send configuration with the RenderContext
            if (ImGui::Button("Render Offline Frame"))
            {
                // THE BLACKBOARD MAGIC: Throw it all in as TRANSIENT variables!
                renderCtx.Set("#OFFLINE_RENDER_TRIGGER", true);
                renderCtx.Set("#OFFLINE_RENDER_SIZE", vec2(targetSize[0], targetSize[1]));
                renderCtx.Set("#OFFLINE_RENDER_PATH", std::string(exportPath));
                renderCtx.Set("#OFFLINE_RENDER_FIT_MODE", currentFitMode);

                if (overrideSettings)
                {
                    renderCtx.Set("#OFFLINE_RENDER_OVERRIDE_SETTINGS", true);
                    renderCtx.Set("#OFFLINE_RENDER_OVERRIDE_EXP", exp);
                    renderCtx.Set("#OFFLINE_RENDER_OVERRIDE_ENV_POW", envPow);
                    renderCtx.Set("#OFFLINE_RENDER_OVERRIDE_ENV_BLUR", envBlr);
                    renderCtx.Set("#OFFLINE_RENDER_OVERRIDE_ENV_ROT", envRt);
                }

                statusMsg = "Render queued for end of frame...";
                statusColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
            }

            if (!statusMsg.empty())
            {
                ImGui::Spacing();
                ImGui::TextColored(statusColor, "%s", statusMsg.c_str());
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void UI::CaptureViewport(RenderContext& renderCtx, const std::string& path)
{
    GLuint tex = renderCtx.Get<GLuint>("POST_PASS_TEX");
    uint32_t w = Renderer::GetViewportWidth();
    uint32_t h = Renderer::GetViewportHeight();
    if (w == 0 || h == 0)
        throw std::runtime_error("Viewport dimensions are zero");

    ImageBuffer buffer;
    buffer.width = w;
    buffer.height = h;
    buffer.channels = 4;
    buffer.pixels.resize(w * h * 4);

    glBindTexture(GL_TEXTURE_2D, tex);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    buffer.SaveToFile(path);
}


} /* namespace blr::core */