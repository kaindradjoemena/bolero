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
            ImGui::SeparatorText("Post Processing");
            {
                float exposure = renderCtx.Get<float>("u_Exposure", 1.0f);
                if (ImGui::DragFloat("Exposure", &exposure, 0.01f, 0.0f, 10.0f, "%.2f EV"))
                    renderCtx.Set("u_Exposure", exposure, Lifetime::PERSISTENT);
            }

            ImGui::Spacing();
            ImGui::SeparatorText("Environment");
            {
                float envBlur = renderCtx.Get<float>("u_EnvironmentBlur", 0.9f);
                if (ImGui::SliderFloat("Skybox Blur", &envBlur, 0.0f, 1.0f, "%.2f"))
                    renderCtx.Set("u_EnvironmentBlur", envBlur, Lifetime::PERSISTENT);
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}


} /* namespace blr::core */