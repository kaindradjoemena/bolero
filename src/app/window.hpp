// window/window.hpp

#pragma once

#include <iostream>
#include <cstdint>
#include <functional>
#include <vector>

struct GLFWwindow;


namespace blr::app
{

class Input;


class Window
{
public:
    static constexpr uint32_t    DEFAULT_WIDTH  = 1920;
    static constexpr uint32_t    DEFAULT_HEIGHT = 720;
    static constexpr const char* DEFAULT_TITLE = "Bolero";

public:
    using ResizeCallback = std::function<void(uint32_t w, uint32_t h)>;

public:
    Window(uint32_t width, uint32_t height, const char* title, Input& input, bool headless = false);
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
    
    GLFWwindow* GetNativeWindow() const { return m_window; }

    Input& GetInput() { return m_input; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }

    void AddResizeCallback(ResizeCallback cb);

    void HandleResize(uint32_t w, uint32_t h);

private:
    bool m_headless;

    GLFWwindow* m_window = nullptr;
    uint32_t m_width     = DEFAULT_WIDTH;
    uint32_t m_height    = DEFAULT_HEIGHT;
    const char* m_title  = DEFAULT_TITLE;

    Input& m_input;

    std::vector<ResizeCallback> m_resizeCallbacks;
};


} /* namespace blr::app */