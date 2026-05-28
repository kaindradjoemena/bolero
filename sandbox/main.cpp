// main.cpp

#include <bolero.hpp>
#include "app/pass/opaque.hpp"  // include the pass

#include <iostream>


#ifdef _WIN32
    extern "C"
    {
        // Forces NVIDIA GPUs
        __declspec(dllexport) uint32_t NvOptimusEnablement = 1;
        
        // Forces AMD GPUs
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
    blr::app::Input input;
    blr::app::Window window(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_TITLE, input);
    window.AddResizeCallback([](uint32_t w, uint32_t h) {
            glViewport(0, 0, w, h);
        });
    
    blr::utils::PrintSystemInfo();

    blr::core::Camera cam;
    cam.SetAspect(window.GetWidth() / window.GetHeight());
    window.AddResizeCallback([&cam](uint32_t w, uint32_t h) {
            cam.SetAspect((float)w / (float)h);
        });
    input.AddMouseScrollCallback([&cam](double xOffset, double yOffset) {
            cam.OnScroll(yOffset);
        });
    input.AddMouseButtonCallback([&input, &cam](int button, int action, int mods) {
                if (button == blr::app::Input::MOUSE_BUTTON_MIDDLE)
                {
                    if (action == blr::app::Input::ACTION_PRESS)
                    {
                        cam.BeginDrag(glm::vec2(input.GetMouseX(), input.GetMouseY()), input.IsKeyDown(blr::app::Input::KEY_L_SHIFT));
                    }
                    else if (action == blr::app::Input::ACTION_RELEASE)
                    {
                        cam.EndDrag();
                    }
                }
            });

    
    blr::core::AssetManager assetManager;
    blr::core::Scene        scene;
    scene.SetCam(&cam);
    
    // Models
    auto shader = assetManager.CreateShader(std::filesystem::path("assets/shaders/renderer_test.glsl"));    
    auto model  = assetManager.CreateModel(std::filesystem::path("assets/models/dude.gltf"), shader);
    blr::core::Transform transform;
    transform.SetPos(blr::core::vec3(0.0f, 0.0f, -5.0f));
    scene.AddEntity(model, transform);
    
    auto model2  = assetManager.CreateModel(std::filesystem::path("assets/models/dude.gltf"), shader);
    blr::core::Transform transform2;
    transform2.SetPos(blr::core::vec3(0.0f, 0.0f, 0.0f));
    scene.AddEntity(model2, transform2);

    // Point light
    blr::core::PointLight pointLight;
    pointLight.position   = blr::core::vec3(2.0f, 2.0f, 2.0f);
    pointLight.range      = 10.0f;
    pointLight.base.power = 10.0f;
    scene.AddLight(pointLight);


    blr::core::Renderer::Init();
    blr::core::RenderPipeline simplePipeline;
    simplePipeline.AddPass(std::make_shared<OpaquePass>());

    while (!window.ShouldClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        window.PollEvents();

        cam.HandleDrag(glm::vec2(input.GetMouseX(), input.GetMouseY()));

        blr::core::Renderer::BeginFrame();
        
        scene.Update(deltaTime, true);

        simplePipeline.Execute(scene);  // pass scene

        blr::core::Renderer::DrawQueue();
        
        window.SwapBuffers();
    }

    blr::core::Renderer::Shutdown();

    return 0;
}
