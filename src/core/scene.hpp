// core/scene.hpp

#pragma once

#include "utils/base.hpp"
#include "core/transform.hpp"

#include <vector>


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

struct Renderable
{
    Ref<Mesh> mesh;
    Ref<Material> material;
    Transform transform;
    bool castShadows = true;
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

    const std::vector<DirLight>& GetDirLights() const { return m_dirLights; }
    const std::vector<PointLight>& GetPointLights() const { return m_pointLights; }
    const std::vector<SpotLight>& GetSpotLights() const { return m_spotLights; }

    void AddEntity(const Ref<Mesh>& mesh, const Ref<Material>& material, const Transform& transform, bool castShadows = true);
    void AddEntity(const Ref<Model>& model, const Transform& transform, bool castShadows = true);
    void AddLight(const DirLight& light);
    void AddLight(const PointLight& light);
    void AddLight(const SpotLight& light);

    void Update(float dt, bool ignoreMsg);

    void SubmitToRenderer();

private:
    Camera* m_cam = nullptr;

    std::vector<Renderable> m_renderables;
    std::vector<DirLight>   m_dirLights;
    std::vector<PointLight> m_pointLights;
    std::vector<SpotLight>  m_spotLights;
};


} /* namespace blr::core */