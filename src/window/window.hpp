// window/window.hpp

#pragma once

#include <GLFW/glfw3.h>

#include <iostream>
#include <cstdint>
#include <functional>
#include <vector>

class Window
{
public:
    using ResizeCallback = std::function<void(uint32_t w, uint32_t h)>;

    Window(int width, int height, const char* title);
    ~Window();

    // Prevent copying
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Prevent moving
    Window(Window&& other) = delete;
    Window& operator=(Window&& other) = delete;

    bool ShouldClose() const;

    void SwapBuffers() const;

    void PollEvents() const;
    
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    const std::vector<ResizeCallback>& GetResizeCallbacks() const { return m_resizeCallbacks; }
    void SetHeight(uint32_t h)  { m_height = h; }
    void SetWidth(uint32_t w) { m_width = w; }

    void AddResizeCallback(ResizeCallback cb);

    void HandleResize(uint32_t w, uint32_t h);

private:
    GLFWwindow* m_window;
    uint32_t m_width, m_height;
    const char* m_title;

    std::vector<ResizeCallback> m_resizeCallbacks;
};
