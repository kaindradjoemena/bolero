// core/material.cpp

#include "material.hpp"

#include "shader.hpp"
#include "texture.hpp"


namespace blr::core
{


Material::Material(Ref<Shader> shader)
: m_shader(shader)
{
}

void Material::Bind() const
{
    if (!m_shader)
        return;

    if (m_albedoMap)
    {
        m_albedoMap->Bind(0); 
        m_shader->SetBool("u_hasAlbedoMap", true);
    }
    else
    {
        m_shader->SetVec3("u_albedoFactor", m_albedoFactor);
        m_shader->SetBool("u_hasAlbedoMap", false);
    }

    if (m_normalMap)
    {
        m_normalMap->Bind(1);
        m_shader->SetBool("u_hasNormalMap", true);
    }
    else
    {
        m_shader->SetBool("u_hasNormalMap", false);
    }

    if (m_metallicMap)
    {
        m_metallicMap->Bind(2);
        m_shader->SetBool("u_hasMetallicMap", true);
    }
    else
    {
        m_shader->SetFloat("u_metallicFactor", m_metallicFactor);
        m_shader->SetBool("u_hasMetallicMap", false);
    }

    if (m_roughnessMap)
    {
        m_roughnessMap->Bind(3);
        m_shader->SetBool("u_hasRoughnessMap", true);
    }
    else
    {
        m_shader->SetFloat("u_roughnessFactor", m_roughnessFactor);
        m_shader->SetBool("u_hasRoughnessMap", false);
    }

    if (m_aoMap)
    {
        m_aoMap->Bind(4);
        m_shader->SetBool("u_hasAOMap", true);
    }
    else
    {
        m_shader->SetFloat("u_AOFactor", m_aoFactor);
        m_shader->SetBool("u_hasAOMap", false);
    }
}


} /* namespace blr::core */