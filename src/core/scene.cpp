// core/scene.cpp

#include "scene.hpp"
#include "core/model.hpp"
#include "core/mesh.hpp"
#include "core/material.hpp"
#include "core/lights.hpp"
#include "core/transform.hpp"
#include "renderer/renderer.hpp"
#include "core/gpu_layout.hpp"

#include <iostream>


namespace blr::core
{


void Scene::AddEntity(const std::string& name, const Ref<Model>& model, const Transform& transform, bool castShadows)
{
    m_entities.push_back({ name, model, transform, castShadows });
}

void Scene::AddLight(const DirLight& light)
{
    m_dirLights.push_back(light);
}

void Scene::AddLight(const PointLight& light)
{
    m_pointLights.push_back(light);
}

void Scene::AddLight(const SpotLight& light)
{
    m_spotLights.push_back(light);
}

void Scene::SubmitToRenderer()
{
    if (!m_cam)
    {
        std::cerr << "Scene::SubmitToRender camera not set!" << std::endl;
        return;
    }
    
    Renderer::UpdateCameraUBO(*m_cam);

    // Directional Lights
    int dirShadowIdx = 0;
    for (const auto& light : m_dirLights)
    {
        int idx = -1;
        if (light.castsShadow && dirShadowIdx < MAX_DIR_LIGHTS)
            idx = dirShadowIdx++;

        Renderer::Submit(light, idx);
    }

    // Spot Lights
    int spotShadowIdx = 0;
    for (const auto& light : m_spotLights)
    {
        int idx = -1;
        if (light.castsShadow && spotShadowIdx < MAX_SPOT_LIGHTS)
            idx = spotShadowIdx++;

        Renderer::Submit(light, idx);
    }

    // Point Lights
    int pointShadowIdx = 0;
    for (const auto& light : m_pointLights)
    {
        int idx = -1;
        if (light.castsShadow && pointShadowIdx < MAX_POINT_LIGHTS)
            idx = pointShadowIdx++;

        Renderer::Submit(light, idx);
    }

    // Flatten entities
    for (const auto& entity : m_entities)
    {
        for (const auto& mesh : entity.model->GetMeshes())
        {
            if (entity.materialOverride)
            {
                Renderer::Submit(mesh, entity.materialOverride, entity.transform);
            }
            else
            {
                Renderer::Submit(mesh, mesh->GetMaterial(), entity.transform);
            }
        }
    }
}


} /* namespace blr::core */