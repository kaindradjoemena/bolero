// core/material.cpp

#include "material.hpp"

#include "core/shader.hpp"
#include "core/texture.hpp"
#include "core/gpu_layout.hpp"


namespace blr::core
{


Material::Material(const Ref<Shader>& shader)
: m_shader(shader)
{
}

void Material::Bind(Shader* overrideShader) const
{
    Shader* targetShader = overrideShader ? overrideShader : m_shader.get();
    if (!targetShader)
        return;

    if (m_albedoMap)
    {
        m_albedoMap->Bind(LayoutLoc::ALBEDO_MAP); 
        targetShader->SetBool("u_hasAlbedoMap", true);
    }
    else
    {
        targetShader->SetVec3("u_albedoFactor", m_albedoFactor);
        targetShader->SetBool("u_hasAlbedoMap", false);
    }

    if (m_normalMap)
    {
        m_normalMap->Bind(LayoutLoc::NORMAL_MAP);
        targetShader->SetBool("u_hasNormalMap", true);
    }
    else
    {
        targetShader->SetBool("u_hasNormalMap", false);
    }

    if (m_metallicMap)
    {
        m_metallicMap->Bind(LayoutLoc::METALLIC_MAP);
        targetShader->SetBool("u_hasMetallicMap", true);
    }
    else
    {
        targetShader->SetFloat("u_metallicFactor", m_metallicFactor);
        targetShader->SetBool("u_hasMetallicMap", false);
    }

    if (m_roughnessMap)
    {
        m_roughnessMap->Bind(LayoutLoc::ROUGHNESS_MAP);
        targetShader->SetBool("u_hasRoughnessMap", true);
    }
    else
    {
        targetShader->SetFloat("u_roughnessFactor", m_roughnessFactor);
        targetShader->SetBool("u_hasRoughnessMap", false);
    }

    if (m_aoMap)
    {
        m_aoMap->Bind(LayoutLoc::AO_MAP);
        targetShader->SetBool("u_hasAOMap", true);
    }
    else
    {
        targetShader->SetFloat("u_AOFactor", m_aoFactor);
        targetShader->SetBool("u_hasAOMap", false);
    }
}


} /* namespace blr::core */