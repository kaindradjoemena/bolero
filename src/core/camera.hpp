// core/camera.hpp

#pragma once

#include "utils/math.hpp"


namespace blr::core
{


class Camera
{
public:
    static constexpr vec3 DEFAULT_UP_DIRECTION = vec3(0.0f, 1.0f, 0.0f);

    static constexpr vec3 DEFAULT_TARGET = vec3(0.0f);

    static constexpr float DEFAULT_FOV      = DegToRadConst(65.0f);
    static constexpr float MIN_FOV          = DegToRadConst(15.0f);
    static constexpr float MAX_FOV          = DegToRadConst(150.0f);

    static constexpr float MIN_DISTANCE     = 0.5f;
    static constexpr float DEFAULT_DISTANCE = 10.0f;
    static constexpr float MAX_DISTANCE     = 1000.0f;

    static constexpr float DEFAULT_NEAR     = 0.01f;
    static constexpr float MIN_NEAR         = 0.001f;
    static constexpr float DEFAULT_FAR      = 100.0f;
    static constexpr float MAX_FAR          = 1000.0f;
    static constexpr float DEFAULT_ASPECT   = 16.0f / 9.0f;

    static constexpr float DEFAULT_ROT_SENS = 0.01f;
    static constexpr float DEFAULT_PAN_SENS = 0.002f;
    static constexpr float DEFAULT_SCR_SENS = 0.1f;

    static constexpr float MIN_PITCH        = DegToRadConst(-89.0f);
    static constexpr float DEFAULT_PITCH    = DegToRadConst(30.0f);
    static constexpr float MAX_PITCH        = DegToRadConst(89.0f);

    static constexpr float DEFAULT_YAW      = DegToRadConst(30.0f);

public:
    Camera() = default;
    ~Camera() = default;
    
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
    float GetDistance() const { return m_distance; }
    vec3  GetTarget()   const { return m_target; }

    float GetRotSense() const { return m_rotSens; }
    float GetPanSense() const { return m_panSens; }
    float GetScrSense() const { return m_scrSens; }

    void SetFov(float fov)  { m_fov = Clamp(fov, MIN_FOV, MAX_FOV); }
    void SetNear(float n)   { m_nearPlane = Clamp(n, MIN_NEAR, m_farPlane); }
    void SetFar(float f)    { m_farPlane = Clamp(f, m_nearPlane, MAX_FAR); }
    void SetAspect(float a) { m_aspect = a; }

    void SetYaw(float y)   { m_yaw = y; }
    void SetPitch(float p) { m_pitch = Clamp(p, MIN_PITCH, MAX_PITCH); }
    void SetDistance(float d)     { m_distance = d; }
    void SetTarget(const vec3& t) { m_target = t; }

    void SetRotSense(float s) { m_rotSens = s; }
    void SetPanSense(float s) { m_panSens = s; }
    void SetScrSense(float s) { m_scrSens = s; }

    vec3 GetPos() const;
    mat4 GetViewMat() const;
    mat4 GetProjMat() const;

    void BeginDrag(const vec2& mousePos, bool isPan);
    void EndDrag();
    void HandleDrag(const vec2& mousePos);
    void OnScroll(double yOffset);

private:
    vec3 m_target = DEFAULT_TARGET;
    vec2 m_lastMousePos = vec2(0.0f);

    float m_distance = DEFAULT_DISTANCE;

    float m_fov       = DEFAULT_FOV;
    float m_nearPlane = DEFAULT_NEAR;
    float m_farPlane  = DEFAULT_FAR;
    float m_aspect    = DEFAULT_ASPECT;

    float m_yaw   = DEFAULT_YAW;
    float m_pitch = DEFAULT_PITCH;

    float m_rotSens = DEFAULT_ROT_SENS;
    float m_panSens = DEFAULT_PAN_SENS;
    float m_scrSens = DEFAULT_SCR_SENS;
    
    bool m_isDragging = false;
    bool m_isPanning  = false;
};


} /* namespace blr::core */