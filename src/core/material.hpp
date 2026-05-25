// core/material.hpp

#pragma once

#include <vector>

#include "types.hpp"


namespace blr::core
{


class Shader;
class Tex;

class Material
{
public:
    Ref<Tex> albedoMap;
    Ref<Tex> normalMap;
    Ref<Tex> metallicMap;
    Ref<Tex> roughnessMap;
    Ref<Tex> aoMap;

    vec3 albedoFactor{1.0f, 1.0f, 1.0f}; 
    float metallicFactor{1.0f};
    float roughnessFactor{0.5f};
    float aoFactor{1.0f};

public:
    ~Material() = default;

    // Prevent copying
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;
    
    // Allow moving
    Material(Material&& other) = default;
    Material& operator=(Material&& other) = default;


    Ref<Shader> GetShader() const { return m_shader; }
    void SetShader(Ref<Shader> shader) { m_shader = shader; }
    
    void Bind() const;

private:
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