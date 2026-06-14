// main.cpp

#include <bolero.hpp>

#include "passes/opaque.hpp"  // light pass
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
    cam.SetAspect((float)window.GetWidth() / (float)window.GetHeight());
    window.AddResizeCallback([&cam](uint32_t w, uint32_t h) {
            cam.SetAspect((float)w / (float)h);
        });
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
    auto model_squaresAndthings = assetManager.CreateModel("bolero://models/squares_and_things.gltf", opaqueShader);
    blrc::Transform transform;
    scene.AddEntity(model_squaresAndthings, transform);

    blrc::Renderer::Init();
    blrc::RenderPipeline forwardRender;

    window.AddResizeCallback([&forwardRender](uint32_t w, uint32_t h) {
            forwardRender.OnResize(w, h);
        });


    // Render pass context
    blrc::RenderContext renderCtx;

    // IBL Skybox Setup Pass
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
    // Opaque Pass (Skybox, Mesh)
    auto skyboxShader = assetManager.CreateShader("bolero://shaders/skybox.glsl");
    blrc::Ref<OpaquePass> opaquePass = std::make_shared<OpaquePass>(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT
                                                                   , opaqueShader, skyboxShader);
    // Post Process Pass
    auto postShader = assetManager.CreateShader("bolero://shaders/post_pass.glsl");
    blrc::Ref<PostPass> postPass = std::make_shared<PostPass>(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, postShader);
    // UI Pass
    blrc::Ref<UIPass> uiPass = std::make_shared<UIPass>(window.GetNativeWindow(), forwardRender.GetPasses());

    // Add Passes to the pipeline
    forwardRender.AddPass(iblPass);
    forwardRender.AddPass(irradiancePass);
    forwardRender.AddPass(prefilterPass);
    forwardRender.AddPass(brdfLutPass);
    forwardRender.AddPass(dirShadowPass);
    forwardRender.AddPass(spotShadowPass);
    forwardRender.AddPass(pointShadowPass);
    forwardRender.AddPass(opaquePass);
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

        cam.HandleDrag(glm::vec2(input.GetMouseX(), input.GetMouseY()));

        blrc::Renderer::BeginFrame();
        
        scene.Update(deltaTime, true);

        renderCtx.ClearTransient();
        forwardRender.Execute(scene, renderCtx);  // pass scene

        window.SwapBuffers();
    }

    blrc::Renderer::Shutdown();

    return 0;
}
