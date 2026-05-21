// core/camera.cpp

#include "camera.hpp"


namespace BLR
{


vec3 Camera::GetPos() const
{
    float yawRad   = DegToRad(m_yaw);
    float pitchRad = DegToRad(m_pitch);

    float x = m_distance * cos(pitchRad) * sin(yawRad);
    float y = m_distance * sin(pitchRad);
    float z = m_distance * cos(pitchRad) * cos(yawRad);

    return m_target + vec3(x, y, z);
}

mat4 Camera::GetViewMat() const
{
    return LookAt(GetPos(), m_target, vec3(0.0f, 1.0f, 0.0f));
}

mat4 Camera::GetProjMat() const
{
    return Perspective(DegToRad(GetFov()), GetAspect(), GetNear(), GetFar());
}

void Camera::BeginDrag(const vec2& mousePos, bool isPan)
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

void Camera::HandleDrag(const vec2& mousePos, const vec2& viewportSize)
{
    vec2 delta = mousePos - m_lastMousePos;

    if (m_isDragging)
    {
        m_yaw   -= delta.x * m_rotSens;
        m_pitch += delta.y * m_rotSens;
        
        m_pitch = Clamp(m_pitch, m_MIN_PITCH, m_MAX_PITCH);
    }

    if (m_isPanning)
    {
        float panScale = m_distance * m_panSens;

        vec3 pos     = GetPos();
        vec3 forward = Norm(m_target - pos);

        vec3 right = Norm(Cross(vec3(0.0f, 1.0f, 0.0f), forward));

        vec3 up = Cross(forward, right);

        m_target += (right * delta.x * panScale);
        m_target += (up * delta.y * panScale);
    }

    m_lastMousePos = mousePos;
}

void Camera::HandleScroll(double yOffset)
{
    float zoomSpeed = m_distance * m_scrSens;
    m_distance -= (float)yOffset * zoomSpeed;

    m_distance = Clamp(m_distance, m_DEFAULT_NEAR, m_DEFAULT_FAR);    // NOTE: Still using magic numbers
}


} /* namespace BLR */