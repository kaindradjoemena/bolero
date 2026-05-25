// core/material.cpp

#include "material.hpp"

#include "shader.hpp"
#include "texture.hpp"

#include <utility>


namespace blr::core
{


Material::Material(Ref<Shader> shader)
    : m_shader(shader) {}

void Material::Bind() const
{
    if (!m_shader)
     return;

    m_shader->Bind();

    // NOTE: might have to reserve some slots for the irradiance and prefilter maps
    // and textures would start at slot 3(?)
    uint32_t slot = 0;

    if (albedoMap)
    {
        albedoMap->Bind(slot);
        m_shader->SetInt("u_albedoMap", slot);
        m_shader->SetBool("u_hasAlbedoMap", true);
        slot++;
    }
    else
    {
        m_shader->SetVec3("u_albedoFactor", albedoFactor);
        m_shader->SetBool("u_hasAlbedoMap", false);
    }

    if (normalMap)
    {
        normalMap->Bind(slot);
        m_shader->SetInt("u_normalMap", slot);
        m_shader->SetBool("u_hasNormalMap", true);
        slot++;
    }
    else
    {
        m_shader->SetBool("u_hasNormalMap", false);
    }

    if (metallicMap)
    {
        metallicMap->Bind(slot);
        m_shader->SetInt("u_metallicMap", slot);
        m_shader->SetBool("u_hasMetallicMap", true);
        slot++;
    }
    else
    {
        m_shader->SetFloat("u_metallicFactor", metallicFactor);
        m_shader->SetBool("u_hasMetallicMap", false);
    }

    if (roughnessMap)
    {
        roughnessMap->Bind(slot);
        m_shader->SetInt("u_roughnessMap", slot);
        m_shader->SetBool("u_hasRoughnessMap", true);
        slot++;
    }
    else
    {
        m_shader->SetFloat("u_roughnessFactor", roughnessFactor);
        m_shader->SetBool("u_hasRoughnessMap", false);
    }

    if (aoMap)
    {
        aoMap->Bind(slot);
        m_shader->SetInt("u_aoMap", slot);
        m_shader->SetBool("u_hasAOMap", true);
        slot++;
    }
    else
    {
        m_shader->SetFloat("u_AOFactor", aoFactor);
        m_shader->SetBool("u_hasAOMap", false);
    }
}

} /* namespace blr::core */