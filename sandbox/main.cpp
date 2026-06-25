// main.cpp

#include <bolero.hpp>

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
    auto opaqueShader = assetManager.CreateShader("bolero://shaders/geometry_pass.glsl");
    auto model = assetManager.CreateModel("bolero://models/squares_and_things.gltf", opaqueShader);
    blrc::Transform transform;
    scene.AddEntity("squares_and_things", model, transform);


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
    blrc::RenderPipeline deferredRender;

    window.AddResizeCallback([&forwardRender, &deferredRender](uint32_t w, uint32_t h) {
            forwardRender.OnWindowResize(w, h);
            deferredRender.OnWindowResize(w, h);
        });

    auto iblPass         = blrc::CreateIBLSetupPass(assetManager);
    auto irradiancePass  = blrc::CreateIrradiancePass(assetManager);
    auto prefilterPass   = blrc::CreatePrefilterPass(assetManager);
    auto brdfLutPass     = blrc::CreateBrdfLutPass(assetManager);

    auto dirShadowPass   = blrc::CreateDirShadowPass(assetManager);
    auto spotShadowPass  = blrc::CreateSpotShadowPass(assetManager);
    auto pointShadowPass = blrc::CreatePointShadowPass(assetManager);

    // Forward Rendering
    auto opaquePass      = blrc::CreateOpauePass(assetManager);
    
    // Deferred Rendering
    auto geometryPass   = blrc::CreateGeometryPass(assetManager);
    auto shadowMaskPass = blrc::CreateShadowMaskPass(assetManager);
    auto lightingPass   = blrc::CreateLightingPass(assetManager);

    auto skyboxPass      = blrc::CreateSkyboxPass(assetManager);
    auto ssaaPass        = blrc::CreateSSAAResolvePass();
    auto postPass        = blrc::CreatePostPass(assetManager);
    
    
    // Offline Rendering
    blrc::RenderPipeline offlineRender;
    offlineRender.AddPass(iblPass);
    offlineRender.AddPass(irradiancePass);
    offlineRender.AddPass(prefilterPass);
    offlineRender.AddPass(brdfLutPass);
    offlineRender.AddPass(dirShadowPass);
    offlineRender.AddPass(spotShadowPass);
    offlineRender.AddPass(pointShadowPass);
    offlineRender.AddPass(geometryPass);
    offlineRender.AddPass(shadowMaskPass);
    offlineRender.AddPass(lightingPass);
    offlineRender.AddPass(skyboxPass);
    offlineRender.AddPass(ssaaPass);
    offlineRender.AddPass(postPass);
    
    // Real Time Deferred Rendering
    deferredRender.AddPass(iblPass);
    deferredRender.AddPass(irradiancePass);
    deferredRender.AddPass(prefilterPass);
    deferredRender.AddPass(brdfLutPass);
    deferredRender.AddPass(dirShadowPass);
    deferredRender.AddPass(spotShadowPass);
    deferredRender.AddPass(pointShadowPass);
    deferredRender.AddPass(geometryPass);
    deferredRender.AddPass(shadowMaskPass);
    deferredRender.AddPass(lightingPass);
    deferredRender.AddPass(skyboxPass);
    deferredRender.AddPass(ssaaPass);
    deferredRender.AddPass(postPass);
    auto uiPassDef = blrc::CreateUIPass(window.GetNativeWindow(), deferredRender.GetPasses());
    deferredRender.AddPass(uiPassDef);


    uiPassDef->DispatchAction([&offlineRender, &scene, &renderCtx]() {
            if (renderCtx.Get<bool>("#OFFLINE_RENDER_TRIGGER", false))
            {
                blrc::vec2 size = renderCtx.Get<blrc::vec2>("#OFFLINE_RENDER_SIZE");
                std::string path = renderCtx.Get<std::string>("#OFFLINE_RENDER_PATH");
                blrc::FitMode fitMode = static_cast<blrc::FitMode>(renderCtx.Get<int>("#OFFLINE_RENDER_FIT_MODE"));

                bool hasOverrides = renderCtx.Get<bool>("#OFFLINE_RENDER_OVERRIDE_SETTINGS", false);
                float prevExp, prevEnvPow, prevEnvBlr, prevEnvRot;

                if (hasOverrides)
                {
                    prevExp    = renderCtx.Get<float>("u_Exposure", 1.0f);
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

                if (hasOverrides)
                {
                    renderCtx.Set("u_Exposure", prevExp, blrc::Lifetime::PERSISTENT);
                    renderCtx.Set("u_EnvironmentPower", prevEnvPow, blrc::Lifetime::PERSISTENT);
                    renderCtx.Set("u_EnvironmentBlur", prevEnvBlr, blrc::Lifetime::PERSISTENT);
                    renderCtx.Set("u_EnvironmentRot", prevEnvRot, blrc::Lifetime::PERSISTENT);
                }
                renderCtx.Set("#OFFLINE_RENDER_TRIGGER", false);
            }
        });


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

        renderCtx.ClearTransient();

        deferredRender.Execute(scene, renderCtx);

        window.SwapBuffers();
    }

    blrc::Renderer::Shutdown();

    return 0;
}
