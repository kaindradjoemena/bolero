// main.cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

// NOTE: put everything inside some include.hpp file instead of this
#include "core/types.hpp"
#include "core/buffer.hpp"
#include "core/vertex_array.hpp"
#include "core/shader.hpp"
#include "core/lights.hpp"
#include "core/camera.hpp"
#include "core/types.hpp"
#include "window/window.hpp"

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
    Window window(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_TITLE);

    window.AddResizeCallback([](uint32_t w, uint32_t h) {
            glViewport(0, 0, w, h);
        });

#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

#ifndef NDEBUG
    int flags; 
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        std::cout << "OpenGL Debug Context initialized!\n";
    }
#endif

    std::cout << "GPU: "            << glGetString(GL_RENDERER) << ", " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLFW Version: "   << glfwGetVersionString() << std::endl;
    std::cout << "GLM Version: "    << GLM_VERSION_MAJOR << "." << GLM_VERSION_MINOR << "." << GLM_VERSION_PATCH << std::endl;

    BLR::Camera cam;
    cam.SetAspect((float)DEFAULT_WINDOW_WIDTH / (float)DEFAULT_WINDOW_HEIGHT);
    cam.SetPitch(20.0f);

    // register window resize callback to change camera aspect
    window.AddResizeCallback([&cam](uint32_t w, uint32_t h) {
            cam.SetAspect((float)w / (float)h);
        });

    float vertices[] = {
        // x, y, z              // nx, ny, nz
        -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
  
        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
  
        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,
  
         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,
  
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,
  
        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f
    };

    // NOTE: std::make_shared<BLR::VertexBuffer>(..) is pretty clunky.
    //       implementing aliasing or some static factory pattern is pretty cool
    auto vbo = std::make_shared<BLR::VertexBuffer>(vertices, sizeof(vertices));
    vbo->SetLayout({
            { BLR::ShaderDataType::Float3, "a_pos" },
            { BLR::ShaderDataType::Float3, "a_norm" }
        });

    // auto ibo = std::make_shared<BLR::IndexBuffer>(indices, 3);

    auto vao = std::make_unique<BLR::VertexArray>();
    vao->AddVertexBuffer(vbo);
    // vao->SetIndexBuffer(ibo);

    BLR::PointLight pointLight;

    pointLight.position   = BLR::vec3(2.0f, 2.0f, 2.0f);
    pointLight.range      = 10.0f;
    pointLight.base.power = 10.0f;

    BLR::Shader shader(std::filesystem::path("assets/shaders/blinn_phong.glsl"));

    glEnable(GL_DEPTH_TEST);

    while (!window.ShouldClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cam.SetYaw(10.0f * currentFrame);   // Rotate camera around the world origin

        BLR::mat4 modelMat = BLR::mat4(1.0f);
        BLR::mat4 viewMat  = cam.GetViewMat();
        BLR::mat4 projMat  = cam.GetProjMat();
        BLR::mat3 normMat  = BLR::Transpose(BLR::Inverse(modelMat));

        shader.Bind();

        shader.SetMat4("u_modelMat", modelMat);
        shader.SetMat4("u_viewMat", viewMat);
        shader.SetMat4("u_projMat", projMat);
        shader.SetMat3("u_normMat", normMat);

        shader.SetVec3("u_lightCol", pointLight.base.color);
        shader.SetFloat("u_lightPow", pointLight.base.power);
        shader.SetVec3("u_lightPos", pointLight.position);
        shader.SetFloat("u_lightRange", pointLight.range);
        shader.SetVec3("u_camPos", cam.GetPos());

        vao->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);

        window.SwapBuffers();
        window.PollEvents();
    }

    return 0;
}
