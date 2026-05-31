// core/material.hpp

#pragma once

#include <vector>

#include "types.hpp"
#include "resource.hpp"


namespace blr::core
{


class Shader;
class Tex;

class Material : public Resource
{
public:
    ~Material() = default;

    // Prevent copying
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;
    
    // Allow moving
    Material(Material&& other) = default;
    Material& operator=(Material&& other) = default;


    Ref<Tex> GetAlbedoMap() const { return m_albedoMap; }
    Ref<Tex> GetNormalMap() const { return m_normalMap; }
    Ref<Tex> GetMetallicMap() const { return m_metallicMap; }
    Ref<Tex> GetRoughnessMap() const { return m_roughnessMap; }
    Ref<Tex> GetAoMap() const { return m_aoMap; }
    vec3 GetAlbedoFactor() const { return m_albedoFactor; } 
    float GetMetallicFactor() const { return m_metallicFactor; }
    float GetRoughnessFactor() const { return m_roughnessFactor; }
    float GetAoFactor() const { return m_aoFactor; }
    Ref<Shader> GetShader() const { return m_shader; }

    void SetAlbedoMap(Ref<Tex> albedoMap) { m_albedoMap = albedoMap; }
    void SetNormalMap(Ref<Tex> normalMap) { m_normalMap = normalMap; }
    void SetMetallicMap(Ref<Tex> metallicMap) { m_metallicMap = metallicMap; }
    void SetRoughnessMap(Ref<Tex> roughnessMap) { m_roughnessMap = roughnessMap; }
    void SetAoMap(Ref<Tex> aoMap) { m_aoMap = aoMap; }
    void SetAlbedoFactor(vec3 albedoFactor) { m_albedoFactor = albedoFactor; } 
    void SetMetallicFactor(float metallicFactor) { m_metallicFactor = metallicFactor; }
    void SetRoughnessFactor(float roughnessFactor) { m_roughnessFactor = roughnessFactor; }
    void SetAoFactor(float aoFactor) { m_aoFactor = aoFactor; }
    void SetShader(Ref<Shader> shader) { m_shader = shader; }
    
    void Bind() const;

private:
    Ref<Tex> m_albedoMap;
    Ref<Tex> m_normalMap;
    Ref<Tex> m_metallicMap;
    Ref<Tex> m_roughnessMap;
    Ref<Tex> m_aoMap;

    vec3 m_albedoFactor{1.0f, 0.0f, 1.0f}; 
    float m_metallicFactor{0.0f};
    float m_roughnessFactor{0.5f};
    float m_aoFactor{1.0f};

    Ref<Shader> m_shader;

// Construction must be called by the AssetManager class through the Create method
friend class AssetManager;
protected:
    Material(Ref<Shader> shader);
    static Ref<Material> Create(Ref<Shader> shader)
    {
        return std::shared_ptr<Material>(new Material(shader));
    }
};


} /* namespace blr::core */