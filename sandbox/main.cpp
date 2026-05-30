// main.cpp

#include <bolero.hpp>
#include "passes/opaque.hpp"  // light pass
#include "passes/shadow.hpp"  // shadow pass

#include <iostream>

namespace blrc = blr::core;
namespace blra = blr::app;


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

    
    blrc::AssetManager assetManager;
    blrc::Scene        scene;
    scene.SetCam(&cam);
    
    // Model
    auto shader = assetManager.CreateShader(std::filesystem::path("assets/shaders/light_pass.glsl"));
    auto model_squaresAndthings = assetManager.CreateModel(std::filesystem::path("assets/models/squares_and_things.gltf"), shader);
    blrc::Transform transform;
    scene.AddEntity(model_squaresAndthings, transform);

    // Directional light
    blrc::DirLight sun;
    sun.direction  = blrc::EulToDir({ -45.0f, 45.0f, 0.0f });
    sun.base.color = blrc::vec3(1.0f, 1.0f, 0.95f);
    scene.AddLight(sun);

    auto depthShader = assetManager.CreateShader("assets/shaders/shadow_pass.glsl");

    blrc::Renderer::Init();
    blrc::RenderPipeline shadowMapping;

    blrc::Ref<ShadowPass> shadowPass = std::make_shared<ShadowPass>(depthShader);
    blrc::Ref<OpaquePass> opaquePass = std::make_shared<OpaquePass>(shader, shadowPass);

    shadowMapping.AddPass(shadowPass);
    shadowMapping.AddPass(opaquePass);


    while (!window.ShouldClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        window.PollEvents();

        cam.HandleDrag(glm::vec2(input.GetMouseX(), input.GetMouseY()));

        blrc::Renderer::BeginFrame();
        
        scene.Update(deltaTime, true);

        shadowMapping.Execute(scene);  // pass scene

        blrc::Renderer::DrawQueue();
        
        window.SwapBuffers();
    }

    blrc::Renderer::Shutdown();

    return 0;
}
