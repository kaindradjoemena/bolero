// core/camera.hpp

#pragma once

#include "types.hpp"

namespace BLR
{


class Camera
{
public:
    Camera() {};
    ~Camera() {};
    
    // Prevent copying
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    // Prevent moving
    Camera(Camera&& other) = delete;
    Camera& operator=(Camera&& other) = delete;

    float GetFov() const { return m_fov; }
    float GetNear() const { return m_nearPlane; }
    float GetFar() const { return m_farPlane; }
    float GetAspect() const { return m_aspect; }

    float GetYaw() const { return m_yaw; }
    float GetPitch() const { return m_pitch; }

    float GetRotSense() const { return m_rotSens; }
    float GetPanSense() const { return m_panSens; }
    float GetScrSense() const { return m_scrSens; }

    void SetFov(float fov)  { m_fov = fov; }
    void SetNear(float n)   { m_nearPlane = n; }
    void SetFar(float f)    { m_farPlane = f; }
    void SetAspect(float a) { m_aspect = a; }

    void SetYaw(float y)   { m_yaw = y; }
    void SetPitch(float p) { m_pitch = p; }

    void SetRotSense(float s) { m_rotSens = s; }
    void SetPanSense(float s) { m_panSens = s; }
    void SetScrSense(float s) { m_scrSens = s; }

    glm::vec3 GetPos() const;
    glm::mat4 GetViewMat() const;
    glm::mat4 GetProjMat() const;

    void BeginDrag(const glm::vec2& mousePos, bool isPan);
    void EndDrag();
    void HandleDrag(const glm::vec2& mousePos, const glm::vec2& viewportSize);
    void HandleScroll(double yOffset);

private:
    glm::vec3 m_target = glm::vec3(0.0f);
    glm::vec2 m_lastMousePos = glm::vec2(0.0f);

    float m_distance = 10.0f;

    float m_fov = 70.0f;    // in degrees
    float m_nearPlane = 0.01f;
    float m_farPlane  = 1000.0f;
    float m_aspect    = 16.0f / 9.0f;
    
    // in degrees
    float m_yaw   = 0.0f;
    float m_pitch = 0.0f;

    float m_rotSens = 0.3f;
    float m_panSens = 0.002f;
    float m_scrSens = 0.1f;
    
    bool m_isDragging = false;
    bool m_isPanning  = false;
};


} /* namespace BLR */