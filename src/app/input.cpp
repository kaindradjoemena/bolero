// app/input.cpp

#include "input.hpp"
#include <iostream>


namespace blr::app
{


bool Input::IsKeyDown(int key) const
{
    return m_keysDown.count(key);
}

bool Input::IsMouseButtonDown(int button) const
{
    return m_mouseButtonsDown.count(button);
}


void Input::OnKey(int key, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        m_keysDown.insert(key);
    }
    else if (action == GLFW_RELEASE)
    {
        m_keysDown.erase(key);
    }

    for (auto& cb : m_keyCallbacks)
        cb(key, action, mods);
}

void Input::OnMouseButton(int button, int action, int mods)
{
    // NOTE: instead of GLFW_*, we can ask the Window class for input(?)
    if (action == GLFW_PRESS)
    {
        m_mouseButtonsDown.insert(button);
    }
    else if (action == GLFW_RELEASE)
    {
        m_mouseButtonsDown.erase(button);
    }

    for (auto& cb : m_mouseButtonCallbacks)
        cb(button, action, mods);
}

void Input::OnMouseScroll(double xoffset, double yoffset)
{
    m_scrollX += xoffset;
    m_scrollY += yoffset;

    for (auto& cb : m_mouseScrollCallbacks)
        cb(xoffset, yoffset);
}

void Input::OnCursorMove(double x, double y)
{
    if (m_firstMouse)
    {
        m_firstMouse = false;
    }
    else
    {
        m_mouseDX += x - m_mouseX;
        m_mouseDY += y - m_mouseY;
    }

    m_mouseX = x;
    m_mouseY = y;
}

void Input::AddKeyCallback(KeyCallback cb)
{
    m_keyCallbacks.push_back(std::move(cb));
}

void Input::AddMouseButtonCallback(MouseButtonCallback cb)
{
    m_mouseButtonCallbacks.push_back(std::move(cb));
}

void Input::AddMouseScrollCallback(MouseScrollCallback cb)
{
    m_mouseScrollCallbacks.push_back(std::move(cb));
}


void Input::EndFrame()
{
    m_mouseDX = m_mouseDY = 0.0;
    m_scrollX = m_scrollY = 0.0;
}


} /* namespace blr::app */