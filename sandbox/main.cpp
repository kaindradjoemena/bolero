// main.cpp

#include <bolero.hpp>

#include "passes/opaque.hpp"  // light pass
#include "passes/skybox.hpp"
#include "passes/ssaa_resolve.hpp"
#include "passes/dir_shadow.hpp"  // shadow pass
#include "passes/spot_shadow.hpp"
#include "passes/point_shadow.hpp"
#include "passes/ibl_setup.hpp"
#include "passes/irradiance.hpp"
#include "passes/prefilter.hpp"
#include "passes/brdf_lut.hpp"
#include "passes/post.hpp"
#include "passes/ui.hpp"

#include <iostream>

namespace blrc = blr::core;
namespace blra = blr::app;

// Force GPU usage
#ifdef _WIN32
    extern "C"
    {
        __declspec(dllexport) uint32_t NvOptimusEnablement = 1;
        
        __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
    }
#endif

constexpr int         DEFAULT_WINDOW_WIDTH  = 1280;
constexpr int         DEFAULT_WINDOW_HEIGHT = 720;
constexpr const char* DEFAULT_WINDOW_TITLE  = "Bolero: Renderer";

float deltaTime = 0.0f;	
float lastFrame = 0.0f;


int main()
{
#ifdef BOLERO_DEV_ASSET_DIR
    blrc::VFS::Mount("bolero://", BOLERO_DEV_ASSET_DIR);
#endif

    blra::Input input;
    blra::Window window(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_TITLE, input);
    window.AddResizeCallback([](uint32_t w, uint32_t h) {
            glViewport(0, 0, w, h);
        });
    
    blr::utils::PrintSystemInfo();

    blrc::Camera cam;
    input.AddMouseScrollCallback([&cam](double xOffset, double yOffset) {
            cam.OnScroll(yOffset);
        });
    input.AddMouseButtonCallback([&input, &cam](int button, int action, int mods) {
                if (button == blra::Input::MOUSE_BUTTON_MIDDLE)
                {
                    if (action == blra::Input::ACTION_PRESS)
                    {
                        cam.BeginDrag(glm::vec2(input.GetMouseX(), input.GetMouseY()), input.IsKeyDown(blra::Input::KEY_L_SHIFT));
                    }
                    else if (action == blra::Input::ACTION_RELEASE)
                    {
                        cam.EndDrag();
                    }
                }
            });

    
    blrc::AssetManager assetManager;
    blrc::Scene        scene;
    scene.SetCam(&cam);
    
    // Model
    auto opaqueShader = assetManager.CreateShader("bolero://shaders/light_pass.glsl");
    auto model = assetManager.CreateModel("bolero://models/squares_and_things.gltf", opaqueShader);
    blrc::Transform transform;
    scene.AddEntity(model, transform);


    // Light
    blrc::DirLight dirLight;
    dirLight.base.power = 0.0f;
    scene.AddLight(dirLight);
    
    blrc::SpotLight spotLight;
    spotLight.base.power = 0.0f;
    scene.AddLight(spotLight);

    blrc::PointLight pointLight;
    pointLight.base.power = 0.0f;
    scene.AddLight(pointLight);


    // Render pass context
    blrc::RenderContext renderCtx;
    blrc::Renderer::Init();
    blrc::RenderPipeline forwardRender;

    window.AddResizeCallback([&forwardRender](uint32_t w, uint32_t h) {
            forwardRender.OnWindowResize(w, h);
        });


    // IBL Skybox Setup Pass
    // auto hdrMap = assetManager.CreateTex("bolero://hdri/citrus_orchard_puresky_2k.hdr");
    auto hdrMap = assetManager.CreateTex("bolero://hdri/newman_cafeteria_2k.hdr");
    auto eqToCubeShader = assetManager.CreateShader("bolero://shaders/equirect_to_cubemap.glsl");
    blrc::Ref<IBLSetupPass> iblPass = std::make_shared<IBLSetupPass>(eqToCubeShader, hdrMap);
    // Scene Irradiance Pass
    auto convolutionShader = assetManager.CreateShader("bolero://shaders/cubemap_convolution.glsl");
    blrc::Ref<IrradiancePass> irradiancePass = std::make_shared<IrradiancePass>(convolutionShader);
    // Environment Map Prefiltering Pass
    auto prefilterShader = assetManager.CreateShader("bolero://shaders/prefilter.glsl");
    blrc::Ref<PrefilterPass> prefilterPass = std::make_shared<PrefilterPass>(prefilterShader);
    // BRDF LUT Pre Computation
    auto brdfLutShader = assetManager.CreateShader("bolero://shaders/brdf_lut.glsl");
    blrc::Ref<BrdfLutPass> brdfLutPass = std::make_shared<BrdfLutPass>(assetManager, brdfLutShader);
    // Scene Depth Pass (Shadow Mapping)
    auto depthShader      = assetManager.CreateShader("bolero://shaders/shadow_pass.glsl");
    auto pointDepthShader = assetManager.CreateShader("bolero://shaders/point_shadow_pass.glsl");
    blrc::Ref<DirShadowPass>   dirShadowPass   = std::make_shared<DirShadowPass>(depthShader);
    blrc::Ref<SpotShadowPass>  spotShadowPass  = std::make_shared<SpotShadowPass>(depthShader);
    blrc::Ref<PointShadowPass> pointShadowPass = std::make_shared<PointShadowPass>(pointDepthShader);
    // Opaque Pass
    blrc::Ref<OpaquePass> opaquePass = std::make_shared<OpaquePass>(opaqueShader);
    // Skybox Pass
    auto skyboxShader = assetManager.CreateShader("bolero://shaders/skybox.glsl");
    blrc::Ref<SkyboxPass> skyboxPass = std::make_shared<SkyboxPass>(skyboxShader);
    // SSAA Pass
    blrc::Ref<SSAAResolvePass> ssaaPass = std::make_shared<SSAAResolvePass>();
    // Post Process Pass
    auto postShader = assetManager.CreateShader("bolero://shaders/post_pass.glsl");
    blrc::Ref<PostPass> postPass = std::make_shared<PostPass>(postShader);
    // UI Pass
    blrc::Ref<UIPass> uiPass = std::make_shared<UIPass>(window.GetNativeWindow(), forwardRender.GetPasses());


    // Offline Rendering
    blrc::RenderPipeline offlineRender;
    offlineRender.AddPass(iblPass);
    offlineRender.AddPass(irradiancePass);
    offlineRender.AddPass(prefilterPass);
    offlineRender.AddPass(brdfLutPass);
    offlineRender.AddPass(dirShadowPass);
    offlineRender.AddPass(spotShadowPass);
    offlineRender.AddPass(pointShadowPass);
    offlineRender.AddPass(opaquePass);
    offlineRender.AddPass(skyboxPass);
    offlineRender.AddPass(ssaaPass);
    offlineRender.AddPass(postPass);

    // Real Time Forward Rendering
    forwardRender.AddPass(iblPass);
    forwardRender.AddPass(irradiancePass);
    forwardRender.AddPass(prefilterPass);
    forwardRender.AddPass(brdfLutPass);
    forwardRender.AddPass(dirShadowPass);
    forwardRender.AddPass(spotShadowPass);
    forwardRender.AddPass(pointShadowPass);
    forwardRender.AddPass(opaquePass);
    forwardRender.AddPass(skyboxPass);
    forwardRender.AddPass(ssaaPass);
    forwardRender.AddPass(postPass);
    forwardRender.AddPass(uiPass);


    float hotReloadTimer = 0.0;
    while (!window.ShouldClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        hotReloadTimer += deltaTime;
        if (hotReloadTimer > 1.0f)
        {
            assetManager.Update();
            hotReloadTimer = 0.0f;
        }

        window.PollEvents();

        // Skip frame when the window is minimized
        if (window.GetHeight() == 0 || window.GetWidth() == 0)
            continue;

        cam.HandleDrag(glm::vec2(input.GetMouseX(), input.GetMouseY()));
        cam.SetAspect((float)blrc::Renderer::GetViewportWidth() / (float)blrc::Renderer::GetViewportHeight());

        blrc::Renderer::BeginFrame();
        
        scene.Update(deltaTime, true);

        renderCtx.ClearTransient();
        forwardRender.Execute(scene, renderCtx);  // pass scene

        // Offline Rendering
        if (renderCtx.Get<bool>("#OFFLINE_RENDER_TRIGGER", false))
        {
            blrc::vec2 size = renderCtx.Get<blrc::vec2>("#OFFLINE_RENDER_SIZE");
            std::string path = renderCtx.Get<std::string>("#OFFLINE_RENDER_PATH");
            blrc::FitMode fitMode = static_cast<blrc::FitMode>(renderCtx.Get<int>("#OFFLINE_RENDER_FIT_MODE"));

            bool hasOverrides = renderCtx.Get<bool>("#OFFLINE_RENDER_OVERRIDE_SETTINGS", false);
            float prevExp, prevEnvPow, prevEnvBlr, prevEnvRot;

            // Apply overrides if they exist
            if (hasOverrides)
            {
                prevExp = renderCtx.Get<float>("u_Exposure", 1.0f);
                prevEnvPow = renderCtx.Get<float>("u_EnvironmentPower", 1.0f);
                prevEnvBlr = renderCtx.Get<float>("u_EnvironmentBlur", 0.9f);
                prevEnvRot = renderCtx.Get<float>("u_EnvironmentRot", 0.0f);

                renderCtx.Set("u_Exposure", renderCtx.Get<float>("#OFFLINE_RENDER_OVERRIDE_EXP"), blrc::Lifetime::PERSISTENT);
                renderCtx.Set("u_EnvironmentPower", renderCtx.Get<float>("#OFFLINE_RENDER_OVERRIDE_ENV_POW"), blrc::Lifetime::PERSISTENT);
                renderCtx.Set("u_EnvironmentBlur", renderCtx.Get<float>("#OFFLINE_RENDER_OVERRIDE_ENV_BLUR"), blrc::Lifetime::PERSISTENT);
                renderCtx.Set("u_EnvironmentRot", renderCtx.Get<float>("#OFFLINE_RENDER_OVERRIDE_ENV_ROT"), blrc::Lifetime::PERSISTENT);
            }

            try
            {
                blrc::ImageBuffer buffer = blrc::FrameCapturer::CapturePipeline(offlineRender, scene, renderCtx, size.x, size.y, fitMode);
                buffer.SaveToFile(path);
                std::cout << "Successfully saved to " << path << "\n";
            }
            catch (const std::exception& e)
            {
                std::cerr << "Offline render failed: " << e.what() << "\n";
            }

            // Put the overrides back for the live editor
            if (hasOverrides)
            {
                renderCtx.Set("u_Exposure", prevExp, blrc::Lifetime::PERSISTENT);
                renderCtx.Set("u_EnvironmentPower", prevEnvPow, blrc::Lifetime::PERSISTENT);
                renderCtx.Set("u_EnvironmentBlur", prevEnvBlr, blrc::Lifetime::PERSISTENT);
                renderCtx.Set("u_EnvironmentRot", prevEnvRot, blrc::Lifetime::PERSISTENT);
            }
        }

        window.SwapBuffers();
    }

    blrc::Renderer::Shutdown();

    return 0;
}
