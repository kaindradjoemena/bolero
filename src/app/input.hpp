// app/input.hpp

#pragma once

#include <GLFW/glfw3.h>

#include <cstdint>
#include <functional>
#include <unordered_set>

namespace blr::app
{
    
    
class Input
{
public:
    // Mouse Buttons
    static constexpr int MOUSE_BUTTON_MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE;
    
    // Keys
    static constexpr int KEY_L_SHIFT = GLFW_KEY_LEFT_SHIFT;

    // Actions
    static constexpr int ACTION_PRESS   = GLFW_PRESS;
    static constexpr int ACTION_RELEASE = GLFW_RELEASE;

    using KeyCallback         = std::function<void(int key, int action, int mods)>;
    using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
    using MouseScrollCallback = std::function<void(double xOffset, double yOffset)>;

public:
    double GetMouseX() const  { return m_mouseX; }
    double GetMouseY() const  { return m_mouseY; }
    double GetMouseDX() const { return m_mouseDX; }
    double GetMouseDY() const { return m_mouseDY; }
    double GetScrollX() const { return m_scrollX; }
    double GetScrollY() const { return m_scrollY; }


    bool IsKeyDown(int key) const;
    bool IsMouseButtonDown(int button) const;

    void OnKey(int key, int action, int mods);
    void OnMouseButton(int button, int action, int mods);
    void OnMouseScroll(double xOffset, double yOffset);
    void OnCursorMove(double x, double y);

    void AddKeyCallback(KeyCallback cb);
    void AddMouseButtonCallback(MouseButtonCallback cb);
    void AddMouseScrollCallback(MouseScrollCallback cb);

    void EndFrame();

private:
    std::unordered_set<int> m_keysDown;
    std::unordered_set<int> m_mouseButtonsDown;

    std::vector<KeyCallback> m_keyCallbacks;
    std::vector<MouseButtonCallback> m_mouseButtonCallbacks;
    std::vector<MouseScrollCallback> m_mouseScrollCallbacks;

    double m_mouseX = 0.0;
    double m_mouseY = 0.0;
    
    double m_mouseDX = 0.0;
    double m_mouseDY = 0.0;
    
    double m_scrollX = 0.0;
    double m_scrollY = 0.0;
    
    bool m_firstMouse = true;
};


} /* namespace blr::app */