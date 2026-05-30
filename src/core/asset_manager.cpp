// core/asset_manager.cpp

#include "asset_manager.hpp"
#include "texture.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "core/uuid.hpp"
#include "utils/debug.hpp"


namespace blr::core
{


AssetManager::AssetManager(const std::filesystem::path& baseDir)
: m_baseDir(baseDir)
{ 
}

Ref<Shader> AssetManager::CreateShader(const std::filesystem::path& filePath)
{
    std::filesystem::path fullPath = m_baseDir / filePath;

    if (m_shaderCache.find(fullPath.string()) != m_shaderCache.end())
        return m_shaderCache[fullPath.string()].shader;

    Ref<Shader> shader = Shader::Create(fullPath);
    shader->SetHandle(UUID::Generate());

#if DEBUG_RESOURCE_CREATION_HANDLE
    std::cout << "AssetManager::CreateShader " << shader->GetHandle() << std::endl;
#endif

    m_shaderCache[fullPath.string()] = { shader, std::filesystem::last_write_time(filePath) };

    return shader;
}

Ref<Tex> AssetManager::CreateTex(const std::filesystem::path& texPath, const TexSpec& texSpec)
{
    Ref<Tex> tex = Tex::Create(m_baseDir / texPath, texSpec);
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
    Ref<Mesh> mesh = Mesh::Create(vertices, indices, material);
    mesh->SetHandle(UUID::Generate());

#if DEBUG_RESOURCE_CREATION_HANDLE
    std::cout << "AssetManager::CreateMesh " << mesh->GetHandle() << std::endl;
#endif    

    return mesh;
}

Ref<Model> AssetManager::CreateModel(const std::filesystem::path& filePath, Ref<Shader> defaultShader)
{
    Ref<Model> model = Model::Create(m_baseDir / filePath, defaultShader, *this);
    model->SetHandle(UUID::Generate());

#if DEBUG_RESOURCE_CREATION_HANDLE
    std::cout << "AssetManager::CreateModel " << model->GetHandle() << std::endl;
#endif
    
    return model;
}

void AssetManager::Update()
{
    for (auto& [pathStr, item] : m_shaderCache)
    {
        try 
        {
            auto currentWriteTime = std::filesystem::last_write_time(pathStr);
            
            if (currentWriteTime > item.lastWrite)
            {
                if (item.shader->Reload())
                {
                    std::cout << "AssetManager::Update " << pathStr << '\n';
                    item.lastWrite = currentWriteTime;
                }
                else
                {
                    item.lastWrite = currentWriteTime; 
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}


} /* namespace blr::core */