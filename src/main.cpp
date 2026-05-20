// main.cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// NOTE: put everything inside some include.hpp file instead of this
#include "core/types.hpp"
#include "core/buffer.hpp"
#include "core/vertex_array.hpp"
#include "core/shader.hpp"
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

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr const char* WINDOW_TITLE = "Bolero: PBR Renderer";


int main()
{
    Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

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

    std::cout << "GPU: " << glGetString(GL_RENDERER) << ", " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLFW Version: " << glfwGetVersionString() << std::endl;
    std::cout << "GLM Version: " << GLM_VERSION_MAJOR << "." << GLM_VERSION_MINOR << "." << GLM_VERSION_PATCH << std::endl;

    float vertices[] = {
        // positions          // colors
        -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // bottom left  - red
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // bottom right - green
         0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,  // top          - blue
    };
    uint32_t indices[] = {0, 1, 2};

    auto vbo = std::make_shared<BLR::VertexBuffer>(vertices, sizeof(vertices));
    vbo->SetLayout({
            { BLR::ShaderDataType::Float3, "a_pos" },
            { BLR::ShaderDataType::Float3, "a_col" }
        });

    auto ibo = std::make_shared<BLR::IndexBuffer>(indices, 3);

    auto vao = std::make_unique<BLR::VertexArray>();
    vao->AddVertexBuffer(vbo);
    vao->SetIndexBuffer(ibo);
    
    BLR::Shader shader(std::filesystem::path("assets/shaders/test.glsl"));

    glEnable(GL_DEPTH_TEST);

    while (!window.ShouldClose())
    {
        glClearColor(0.8f, 0.0f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
        glm::mat4 mvp = projection * view * model;

        shader.Bind();
        shader.SetMat4("u_mvp", mvp);
        vao->Bind();
        glDrawElements(GL_TRIANGLES, vao->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

        window.SwapBuffers();
        window.PollEvents();
    }

    return 0;
}
