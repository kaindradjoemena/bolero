// window/window.cpp

#include "window.hpp"
#include "input.hpp"

#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdint>


namespace blr::app
{


static void FramebufferSizeCallback(GLFWwindow* window, int w, int h)
{
    auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win)
    {
        win->HandleResize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
    }
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win)
    {
        win->GetInput().OnKey(key, action, mods);
    }
}

static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win)
    {
        win->GetInput().OnMouseButton(button, action, mods);
    }
}

static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win)
    {
        win->GetInput().OnMouseScroll(xoffset, yoffset);
    }
}

static void CursorPosCallback(GLFWwindow* window, double x, double y)
{
    auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win)
    {
        win->GetInput().OnCursorMove(x, y);
    }
}


Window::Window(uint32_t width, uint32_t height, const char* title, Input& input)
    : m_input(input), m_width(width), m_height(height), m_title(title)
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
    glfwSetKeyCallback(m_window, KeyCallback);
    glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    glfwSetScrollCallback(m_window, ScrollCallback);
    glfwSetCursorPosCallback(m_window, CursorPosCallback);
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


void Window::AddResizeCallback(ResizeCallback cb)
{
    m_resizeCallbacks.push_back(std::move(cb));
}

void Window::HandleResize(uint32_t w, uint32_t h)
{
    m_width  = w;
    m_height = h;

    for (auto& cb : m_resizeCallbacks)
    {
        cb(w, h);
    }
}


} /* namespace blr::app */