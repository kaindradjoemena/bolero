// core/scene.cpp

#include "scene.hpp"
#include "core/model.hpp"
#include "core/mesh.hpp"
#include "core/material.hpp"
#include "core/lights.hpp"
#include "core/transform.hpp"
#include "renderer/renderer.hpp"

#include <iostream>


namespace blr::core
{

void Scene::AddEntity(const Ref<Mesh>& mesh, const Ref<Material>& material, const Transform& transform, bool castShadows)
{
    m_renderables.push_back({ mesh, material, transform, castShadows });
}

void Scene::AddEntity(const Ref<Model>& model, const Transform& transform, bool castShadows)
{
    for (const auto& mesh : model->GetMeshes())
        m_renderables.push_back({ mesh, mesh->GetMaterial(), transform, castShadows });
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

void Scene::Update(float dt, bool ignoreMsg)
{
    if (!ignoreMsg)
        std::cout << "Scene::Update(float dt, bool ignoreMsg) is empty. Update(dt, true) to ignore" << std::endl;
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
        if (light.castsShadow && dirShadowIdx < 4)
            idx = dirShadowIdx++;

        Renderer::Submit(light, idx);
    }
    // Point Lights
    int pointShadowIdx = 0;
    for (const auto& light : m_pointLights)
    {
        int idx = -1;
        if (light.castsShadow && pointShadowIdx < 4)
            idx = pointShadowIdx++;

        Renderer::Submit(light, idx);
    }
    // Spot Lights
    int spotShadowIdx = 0;
    for (const auto& light : m_spotLights)
    {
        int idx = -1;
        if (light.castsShadow && spotShadowIdx < 4)
            idx = spotShadowIdx++;

        Renderer::Submit(light, idx);
    }

    // Renderables
    for (const auto& renderable : m_renderables)
        Renderer::Submit(renderable.mesh, renderable.material, renderable.transform);
}


} /* namespace blr::core */