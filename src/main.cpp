// main.cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assimp/version.h>

#include <iostream>

// NOTE: put everything inside some include.hpp file instead of this
#include "core/types.hpp"
#include "core/wrappers/buffer.hpp"
#include "core/wrappers/vertex_array.hpp"
#include "core/shader.hpp"
#include "core/model.hpp"
#include "core/mesh.hpp"
#include "core/material.hpp"
#include "core/lights.hpp"
#include "core/camera.hpp"
#include "core/types.hpp"
#include "core/asset_manager.hpp"
#include "renderer/renderer.hpp"

#include "app/window.hpp"
#include "app/input.hpp"

#include "utils/debug.hpp"

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
    blr::core::AssetManager assetManager;
    blr::app::Input input;

    blr::app::Window window(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_TITLE, input);

    window.AddResizeCallback([](uint32_t w, uint32_t h) {
            glViewport(0, 0, w, h);
        });

    blr::core::Renderer::Init();

    std::cout << "GPU: "            << glGetString(GL_RENDERER) << ", " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLFW Version: "   << glfwGetVersionString() << std::endl;
    std::cout << "GLM Version: "    << GLM_VERSION_MAJOR << "." << GLM_VERSION_MINOR << "." << GLM_VERSION_PATCH << std::endl;
    std::cout << "Assimp Version: " << aiGetVersionMajor() << "." << aiGetVersionMinor() << "." << aiGetVersionPatch() << std::endl;

    blr::core::Camera cam;
    cam.SetAspect((float)DEFAULT_WINDOW_WIDTH / (float)DEFAULT_WINDOW_HEIGHT);

    // camera aspect
    window.AddResizeCallback([&cam](uint32_t w, uint32_t h) {
            cam.SetAspect((float)w / (float)h);
        });
    
    // zooming
    input.AddMouseScrollCallback([&cam](double xOffset, double yOffset) {
            cam.OnScroll(yOffset);
        });

    // shift and middle mouse for dragging
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

    blr::core::PointLight pointLight;
    pointLight.position   = blr::core::vec3(2.0f, 2.0f, 2.0f);
    pointLight.range      = 10.0f;
    pointLight.base.power = 10.0f;

    auto shader = assetManager.CreateShader(std::filesystem::path("assets/shaders/renderer_test.glsl"));

    auto model = assetManager.CreateModel(std::filesystem::path("assets/models/dude.gltf"), shader);
    
    blr::core::Transform myEntityTransform;
    myEntityTransform.SetPos(blr::core::vec3(0.0f, 0.0f, -5.0f));

    glEnable(GL_DEPTH_TEST);

    while (!window.ShouldClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myEntityTransform.SetRot(glm::rotate(myEntityTransform.GetRot(), 0.01f, blr::core::vec3(0.0f, 1.0f, 0.0f)));

        blr::core::Renderer::BeginScene(cam);

        shader->Bind();
        shader->SetMat4("u_ViewProj", cam.GetProjMat() * cam.GetViewMat());

        for (const auto& mesh : model->GetMeshes())
        {
            blr::core::Renderer::Submit(mesh, mesh->GetMaterial(), myEntityTransform);
        }

        blr::core::Renderer::Render();

        window.SwapBuffers();
        window.PollEvents();

        // Update camera
        cam.HandleDrag(glm::vec2(input.GetMouseX(), input.GetMouseY()));
    }

    blr::core::Renderer::Shutdown();

    return 0;
}
