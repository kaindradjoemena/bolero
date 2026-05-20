// window/window.cpp

#include "window.hpp"

#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdint>

static void FramebufferSizeCallback(GLFWwindow* window, int w, int h)
{
    auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win)
    {
        win->HandleResize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
    
        for (auto& cb : win->GetResizeCallbacks())
        {
            cb(win->GetWidth(), win->GetHeight());
        }
    }
    // glViewport(0, 0, width, height);
}

Window::Window(int width, int height, const char* title)
    : m_width(width), m_height(height), m_title(title)
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(m_width, m_height, m_title, nullptr, nullptr);
        if (!m_window)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(m_window);

        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);
}

Window::~Window()
{
    if (m_window)
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
}

bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(m_window);
}
    
void Window::SwapBuffers() const
{
    glfwSwapBuffers(m_window);
}

void Window::PollEvents() const
{
    glfwPollEvents();
}

void Window::HandleResize(uint32_t w, uint32_t h)
{
    m_width  = w;
    m_height = h;
}

void Window::AddResizeCallback(ResizeCallback cb)
{
    m_resizeCallbacks.push_back(std::move(cb));
}