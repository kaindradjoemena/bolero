// core/scene.hpp

#pragma once

#include "utils/base.hpp"
#include "core/transform.hpp"

#include <vector>
#include <string>


namespace blr::core
{


class Model;
class Camera;
class Mesh;
class Material;
class Transform;
struct DirLight;
struct PointLight;
struct SpotLight;

struct Entity
{
    std::string name;
    Ref<Model> model;
    Transform transform;
    bool castShadows = true;

    Ref<Material> materialOverride = nullptr;
};


class Scene
{
public:
    Scene() = default;
    ~Scene() = default;

    // Prevent copying
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    // Allow moving
    Scene(Scene&&) = default;
    Scene& operator=(Scene&&) = default;


    void SetCam(Camera* camera) { m_cam = camera; }
    Camera* GetCam() const { return m_cam; }

    std::vector<Entity>& GetEntity() { return m_entities; }
    std::vector<DirLight>& GetDirLights() { return m_dirLights; }
    std::vector<PointLight>& GetPointLights() { return m_pointLights; }
    std::vector<SpotLight>& GetSpotLights() { return m_spotLights; }

    void AddEntity(const std::string& name, const Ref<Model>& model, const Transform& transform, bool castShadows = true);
    void AddLight(const DirLight& light);
    void AddLight(const PointLight& light);
    void AddLight(const SpotLight& light);

    void SubmitToRenderer();

private:
    Camera* m_cam = nullptr;

    std::vector<Entity>     m_entities;
    std::vector<DirLight>   m_dirLights;
    std::vector<PointLight> m_pointLights;
    std::vector<SpotLight>  m_spotLights;
};


} /* namespace blr::core */