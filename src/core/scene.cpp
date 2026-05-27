// core/scene.cpp

#include "scene.hpp"
#include "core/model.hpp"
#include "core/mesh.hpp"
#include "core/material.hpp"
#include "renderer/renderer.hpp"

#include <iostream>


namespace blr::core
{

void Scene::AddEntity(const Ref<Mesh>& mesh, const Ref<Material>& material, const Transform& transform)
{
    m_renderables.push_back({ mesh, material, transform });
}

void Scene::AddEntity(const Ref<Model>& model, const Transform& transform)
{
    for (const auto& mesh : model->GetMeshes())
        m_renderables.push_back({ mesh, mesh->GetMaterial(), transform });
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

    for (const auto& light : m_dirLights)
        Renderer::Submit(light);
    for (const auto& light : m_pointLights)
        Renderer::Submit(light);
    for (const auto& light : m_spotLights)
        Renderer::Submit(light);

    for (const auto& renderable : m_renderables)
        Renderer::Submit(renderable.mesh, renderable.material, renderable.transform);
}


} /* namespace blr::core */