// main.cpp

#include <bolero.hpp>
#include "passes/opaque.hpp"  // light pass
#include "passes/shadow.hpp"  // shadow pass
#include "passes/post.hpp"

#include <iostream>

namespace blrc = blr::core;
namespace blra = blr::app;


#define XSTR(s) STR(s)
#define STR(s) #s

#ifdef SANDBOX_ROOT_DIR
    const std::string ROOT_DIR = XSTR(SANDBOX_ROOT_DIR);
#else
    const std::string ROOT_DIR = "";
#endif

constexpr int         DEFAULT_WINDOW_WIDTH  = 1280;
constexpr int         DEFAULT_WINDOW_HEIGHT = 720;
constexpr const char* DEFAULT_WINDOW_TITLE  = "Bolero: Renderer";

float deltaTime = 0.0f;	
float lastFrame = 0.0f;


int main()
{
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

    
    blrc::AssetManager assetManager(ROOT_DIR);
    blrc::Scene        scene;
    scene.SetCam(&cam);
    
    // Model
    auto opaqueShader = assetManager.CreateShader(std::filesystem::path("assets/shaders/light_pass.glsl"));
    auto model_squaresAndthings = assetManager.CreateModel(std::filesystem::path("assets/models/squares_and_things.gltf"), opaqueShader);
    blrc::Transform transform;
    scene.AddEntity(model_squaresAndthings, transform);

    // Directional light
    blrc::DirLight sun;
    sun.direction  = blrc::EulToDir({ -45.0f, 45.0f, 0.0f });
    sun.base.color = blrc::vec3(1.0f, 1.0f, 0.95f);
    scene.AddLight(sun);

    blrc::Renderer::Init();
    blrc::RenderPipeline shadowMapping;

    window.AddResizeCallback([&shadowMapping](uint32_t w, uint32_t h) {
            shadowMapping.OnResize(w, h);
        });

    // Render Passes
    auto depthShader = assetManager.CreateShader("assets/shaders/shadow_pass.glsl");
    blrc::Ref<ShadowPass> shadowPass = std::make_shared<ShadowPass>(depthShader);
    
    blrc::Ref<OpaquePass> opaquePass = std::make_shared<OpaquePass>(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, opaqueShader, shadowPass);
    
    auto postShader = assetManager.CreateShader("assets/shaders/post_pass.glsl");
    blrc::Ref<PostPass>   postPass   = std::make_shared<PostPass>(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, postShader, opaquePass);

    // Add Passes to the pipeline
    shadowMapping.AddPass(shadowPass);
    shadowMapping.AddPass(opaquePass);
    shadowMapping.AddPass(postPass);


    float hotReloadTimer = 0.0;
    while (!window.ShouldClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        hotReloadTimer += deltaTime;
        if (hotReloadTimer > 1.0f)
        {
            std::cout << "checking for updates" << "\n";
            assetManager.Update();
            hotReloadTimer = 0.0f;
        }

        window.PollEvents();

        cam.HandleDrag(glm::vec2(input.GetMouseX(), input.GetMouseY()));

        blrc::Renderer::BeginFrame();
        
        scene.Update(deltaTime, true);

        shadowMapping.Execute(scene);  // pass scene

        for (const auto& pass : shadowMapping.GetPasses())
        {
            const auto& passStats = pass->GetStats();
            std::cout << "  [" << pass->GetName() << "] " 
                      << " CPU: " << passStats.cpuTimeMs << "ms"
                      << " | GPU: " << passStats.gpuTimeMs << "ms"
                      << " | Draws: " << passStats.drawCalls << "\n";
        }

        window.SwapBuffers();
    }

    blrc::Renderer::Shutdown();

    return 0;
}
