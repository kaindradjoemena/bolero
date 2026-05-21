// core/camera.hpp

#pragma once

#include "types.hpp"

namespace BLR
{


class Camera
{
public:
    static constexpr float m_DEFAULT_FOV      = 65.0f;
    static constexpr float m_MIN_FOV          = 15.0f;
    static constexpr float m_MAX_FOV          = 150.0f;

    static constexpr float m_DEFAULT_DISTANCE = 10.0f;

    static constexpr float m_DEFAULT_NEAR     = 0.001f;
    static constexpr float m_MIN_NEAR         = 0.01f;
    static constexpr float m_DEFAULT_FAR      = 100.0f;
    static constexpr float m_MAX_FAR          = 1000.0f;
    static constexpr float m_DEFAULT_ASPECT   = 16.0f / 9.0f;

    static constexpr float m_DEFAULT_ROT_SENS = 0.3f;
    static constexpr float m_DEFAULT_PAN_SENS = 0.002f;
    static constexpr float m_DEFAULT_SCR_SENS = 0.1f;

    static constexpr float m_MIN_PITCH        = -89.0f;
    static constexpr float m_MAX_PITCH        = 89.0f;

public:
    Camera() {};
    ~Camera() {};
    
    // Prevent copying
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    // Prevent moving
    Camera(Camera&& other) = delete;
    Camera& operator=(Camera&& other) = delete;

    float GetFov()    const { return m_fov; }
    float GetNear()   const { return m_nearPlane; }
    float GetFar()    const { return m_farPlane; }
    float GetAspect() const { return m_aspect; }

    float GetYaw()   const { return m_yaw; }
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

    vec3 GetPos() const;
    mat4 GetViewMat() const;
    mat4 GetProjMat() const;

    void BeginDrag(const vec2& mousePos, bool isPan);
    void EndDrag();
    void HandleDrag(const vec2& mousePos, const vec2& viewportSize);
    void HandleScroll(double yOffset);

private:
    vec3 m_target = vec3(0.0f);
    vec2 m_lastMousePos = vec2(0.0f);

    float m_distance = m_DEFAULT_DISTANCE;

    float m_fov       = m_DEFAULT_FOV;    // in degrees
    float m_nearPlane = m_DEFAULT_NEAR;
    float m_farPlane  = m_DEFAULT_FAR;
    float m_aspect    = m_DEFAULT_ASPECT;
    
    // in degrees
    float m_yaw   = 0.0f;
    float m_pitch = 0.0f;

    float m_rotSens = m_DEFAULT_ROT_SENS;
    float m_panSens = m_DEFAULT_PAN_SENS;
    float m_scrSens = m_DEFAULT_SCR_SENS;
    
    bool m_isDragging = false;
    bool m_isPanning  = false;
};


} /* namespace BLR */