// core/transform.hpp

#pragma once

#include "utils/math.hpp"


namespace blr::core
{


struct Transform
{
public:
    vec3 GetPos() const { return m_pos; }
    vec3 GetScl() const { return m_scl; }
    quat GetRot() const { return m_rot; }
    mat4 GetModelMat() const
    {
        if (!m_isDirty)
            return m_modelMat;

        mat4 modelMat = mat4(1.0f);
        modelMat      = glm::translate(modelMat, m_pos);
        modelMat     *= glm::mat4_cast(m_rot);
        modelMat      = glm::scale(modelMat, m_scl);
        
        m_isDirty = false;
        m_modelMat = modelMat;

        return m_modelMat;
    }

    void SetPos(vec3 pos) { m_pos = pos; m_isDirty = true; }
    void SetScl(vec3 scl) { m_scl = scl; m_isDirty = true; }
    void SetRot(quat rot) { m_rot = rot; m_isDirty = true; }

private:
    mutable bool m_isDirty = false;
    
    vec3 m_pos = vec3(0.0f);
    vec3 m_scl = vec3(1.0f);
    quat m_rot = quat(1.0f, 0.0f, 0.0f, 0.0f);
    mutable mat4 m_modelMat = mat4(1.0f);
};


} /* namespace blr::core */