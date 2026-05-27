// core/asset_manager.cpp

#include "asset_manager.hpp"
#include "texture.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "utils/uuid.hpp"
#include "utils/debug.hpp"


namespace blr::core
{


Ref<Shader> AssetManager::CreateShader(const std::filesystem::path& filePath)
{
    Ref<Shader> shader = Shader::Create(filePath);
    shader->SetHandle(UUID::Generate());

#if DEBUG_RESOURCE_CREATION_HANDLE
    std::cout << "AssetManager::CreateShader " << shader->GetHandle() << std::endl;
#endif

    return shader;
}

Ref<Tex> AssetManager::CreateTex(const std::filesystem::path& texPath, const TexSpec& texSpec)
{
    Ref<Tex> tex = Tex::Create(texPath, texSpec);
    tex->SetHandle(UUID::Generate());

#if DEBUG_RESOURCE_CREATION_HANDLE
    std::cout << "AssetManager::CreateTex " << tex->GetHandle() << std::endl;
#endif

    return tex;
}

Ref<Tex> AssetManager::CreateTex(const TexSpec& texSpec)
{
    Ref<Tex> tex = Tex::Create(texSpec);
    tex->SetHandle(UUID::Generate());

#if DEBUG_RESOURCE_CREATION_HANDLE
    std::cout << "AssetManager::CreateTex " << tex->GetHandle() << std::endl;
#endif    
    
    return tex;
}

Ref<Material> AssetManager::CreateMaterial(Ref<Shader> shader)
{
    Ref<Material> mat = Material::Create(shader);
    mat->SetHandle(UUID::Generate());

#if DEBUG_RESOURCE_CREATION_HANDLE
    std::cout << "AssetManager::CreateMaterial " << mat->GetHandle() << std::endl;
#endif

    return mat;
}

Ref<Mesh> AssetManager::CreateMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Ref<Material> material)
{
    Ref<Mesh> mesh = Mesh::Create(vertices, indices, material, *this);
    mesh->SetHandle(UUID::Generate());

#if DEBUG_RESOURCE_CREATION_HANDLE
    std::cout << "AssetManager::CreateMesh" << mesh->GetHandle() << std::endl;
#endif    

    return mesh;
}

Ref<Model> AssetManager::CreateModel(const std::filesystem::path& filePath, Ref<Shader> defaultShader)
{
    Ref<Model> model = Model::Create(filePath, defaultShader, *this);
    model->SetHandle(UUID::Generate());

#if DEBUG_RESOURCE_CREATION_HANDLE
    std::cout << "AssetManager::CreateModel " << model->GetHandle() << std::endl;
#endif
    
    return model;
}


} /* namespace blr::core */