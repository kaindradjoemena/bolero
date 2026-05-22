// core/camera.cpp

#include "camera.hpp"


namespace BLR
{


vec3 Camera::GetPos() const
{
    float x = m_distance * cos(m_pitch) * sin(m_yaw);
    float y = m_distance * sin(m_pitch);
    float z = m_distance * cos(m_pitch) * cos(m_yaw);

    return m_target + vec3(x, y, z);
}

mat4 Camera::GetViewMat() const
{
    return LookAt(GetPos(), m_target, DEFAULT_UP_DIRECTION);
}

mat4 Camera::GetProjMat() const
{
    return Perspective(GetFov(), GetAspect(), GetNear(), GetFar());
}

void Camera::BeginDrag(const vec2& mousePos, bool isPan)
{
    m_lastMousePos = mousePos;
    if (isPan)
    {
        m_isPanning = true;
        m_isDragging = false;
    }
    else
    {
        m_isDragging = true;
        m_isPanning = false;
    }
}

void Camera::EndDrag()
{
    m_isDragging = false;
    m_isPanning  = false;
}

void Camera::HandleDrag(const vec2& mousePos)
{
    vec2 delta = mousePos - m_lastMousePos;

    if (m_isDragging)
    {
        m_yaw   -= delta.x * m_rotSens;
        m_pitch += delta.y * m_rotSens;
        
        m_pitch = Clamp(m_pitch, MIN_PITCH, MAX_PITCH);
    }

    if (m_isPanning)
    {
        float panScale = m_distance * m_panSens;

        vec3 pos     = GetPos();
        vec3 forward = Norm(m_target - pos);

        vec3 right = Norm(Cross(DEFAULT_UP_DIRECTION, forward));

        vec3 up = Cross(forward, right);

        m_target += (right * delta.x * panScale);
        m_target += (up * delta.y * panScale);
    }

    m_lastMousePos = mousePos;
}

void Camera::OnScroll(double yOffset)
{
    float zoomSpeed = m_distance * m_scrSens;
    m_distance -= (float)yOffset * zoomSpeed;

    m_distance = Clamp(m_distance, MIN_DISTANCE, MAX_DISTANCE);
}


} /* namespace BLR */