// utils/math.cpp

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blr::core
{


using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using quat = glm::quat;

inline
float Clamp(float x, float min, float max) { return glm::clamp(x, min, max); }

inline
float DegToRad(float deg) { return glm::radians(deg); }

inline constexpr
float DegToRadConst(float deg) { return glm::radians(deg); }

inline
vec3 QuatToEul(quat q) { return glm::eulerAngles(q); }

inline
quat EulToQuat(vec3 e) { return quat(e); }

inline
vec3 EulToDir(vec3 e) { return glm::normalize(glm::quat(glm::radians(e)) * glm::vec3(0.0f, 0.0f, -1.0f)); }

inline
vec3 Norm(vec3 v) { return glm::normalize(v); }

inline
vec3 Cross(vec3 a, vec3 b) { return glm::cross(a, b); }

inline
mat3 Transpose(const mat3& m) { return glm::transpose(m); } 

inline
mat4 Transpose(const mat4& m) { return glm::transpose(m); }

inline
mat3 Inverse(const mat3& m) { return glm::inverse(m); }

inline
mat4 Inverse(const mat4& m) { return glm::inverse(m); }

inline
mat4 LookAt(const vec3& eye, const vec3& center, const vec3& up) { return glm::lookAt(eye, center, up); }

inline
mat4 Perspective(float rad, float aspect, float near, float far) { return glm::perspective(rad, aspect, near, far); }

inline
mat4 Ortho(float left, float right, float bottom, float top, float near, float far) { return glm::ortho(left, right, bottom, top, near, far); }


} /* namespace blr::core */