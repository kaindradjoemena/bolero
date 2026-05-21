// core/camera.cpp

#include "camera.hpp"


namespace BLR
{


glm::vec3 Camera::GetPos() const
{
    float yawRad   = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);

    float x = m_distance * cos(pitchRad) * sin(yawRad);
    float y = m_distance * sin(pitchRad);
    float z = m_distance * cos(pitchRad) * cos(yawRad);

    return m_target + glm::vec3(x, y, z);
}

glm::mat4 Camera::GetViewMat() const
{
    return glm::lookAt(GetPos(), m_target, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::GetProjMat() const
{
    return glm::perspective(glm::radians(GetFov()), GetAspect(), GetNear(), GetFar());
}

void Camera::BeginDrag(const glm::vec2& mousePos, bool isPan)
{
    m_lastMousePos = mousePos;
    m_isDragging = !isPan;
    m_isPanning  = isPan;
}

void Camera::EndDrag()
{
    m_isDragging = false;
    m_isPanning  = false;
}

void Camera::HandleDrag(const glm::vec2& mousePos, const glm::vec2& viewportSize)
{
    glm::vec2 delta = mousePos - m_lastMousePos;

    if (m_isDragging)
    {
        m_yaw   -= delta.x * m_rotSens;
        m_pitch += delta.y * m_rotSens;
        
        m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
    }

    if (m_isPanning)
    {
        float panScale = m_distance * m_panSens;

        glm::vec3 pos     = GetPos();
        glm::vec3 forward = glm::normalize(m_target - pos);

        glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), forward));

        glm::vec3 up = glm::cross(forward, right);

        m_target += (right * delta.x * panScale);
        m_target += (up * delta.y * panScale);
    }

    m_lastMousePos = mousePos;
}

void Camera::HandleScroll(double yOffset)
{
    float zoomSpeed = m_distance * m_scrSens;
    m_distance -= (float)yOffset * zoomSpeed;

    m_distance = glm::clamp(m_distance, 0.01f, 1000.0f);    // NOTE: Still using magic numbers
}


} /* namespace BLR */