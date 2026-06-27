// core/asset_manager.cpp

#include "asset_manager.hpp"
#include "texture.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "uuid.hpp"
#include "vfs.hpp"
#include "utils/debug.hpp"


namespace blr::core
{


Ref<Shader> AssetManager::CreateShader(std::string_view filePath)
{
    std::filesystem::path physPath = VFS::Resolve(filePath);

    if (m_shaderCache.find(physPath.string()) != m_shaderCache.end())
        return m_shaderCache[physPath.string()].shader;

    Ref<Shader> shader = Shader::Create(physPath);
    shader->SetHandle(UUID::Generate());

#if DEBUG_RESOURCE_CREATION_HANDLE
    std::cout << "AssetManager::CreateShader " << shader->GetHandle() << std::endl;
#endif

    m_shaderCache[physPath.string()] = { shader, std::filesystem::last_write_time(physPath) };

    return shader;
}

Ref<Tex> AssetManager::CreateTex(std::string_view texPath, const TexSpec& texSpec)
{
    std::filesystem::path physPath = VFS::Resolve(texPath);

    if (m_texCache.find(physPath.string()) != m_texCache.end())
        return m_texCache[physPath.string()];

    Ref<Tex> tex = Tex::Create(physPath, texSpec);
    tex->SetHandle(UUID::Generate());

    m_texCache[physPath.string()] = tex;

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

Ref<Material> AssetManager::CreateMaterial(const Ref<Shader>& shader)
{
    Ref<Material> mat = Material::Create(shader);
    mat->SetHandle(UUID::Generate());

#if DEBUG_RESOURCE_CREATION_HANDLE
    std::cout << "AssetManager::CreateMaterial " << mat->GetHandle() << std::endl;
#endif

    return mat;
}

Ref<Mesh> AssetManager::CreateMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, const Ref<Material>& material)
{
    Ref<Mesh> mesh = Mesh::Create(vertices, indices, material);
    mesh->SetHandle(UUID::Generate());

#if DEBUG_RESOURCE_CREATION_HANDLE
    std::cout << "AssetManager::CreateMesh " << mesh->GetHandle() << std::endl;
#endif    

    return mesh;
}

Ref<Model> AssetManager::CreateModel(std::string_view filePath, const Ref<Shader>& defaultShader)
{
    std::filesystem::path physPath = VFS::Resolve(filePath);

    Ref<Model> model = Model::Create(physPath, defaultShader, *this);
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