// core/model.hpp

#pragma once

#include "utils/base.hpp"
#include "resource.hpp"

#include <vector>
#include <string>
#include <filesystem>

struct aiNode;
struct aiMesh;
struct aiScene;


namespace blr::core
{


class Mesh;
class Material;
class Shader;
class AssetManager;


class Model : public Resource
{
public:
    ~Model() = default;

    // Prevent copying
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    // Allow moving
    Model(Model&& other) = default;
    Model& operator=(Model&& other) = default;


    const std::vector<Ref<Mesh>>& GetMeshes() const { return m_meshes; }

private:
    void ProcessMaterials(const aiScene* scene, AssetManager& assetManager);
    void ProcessNode(aiNode* node, const aiScene* scene, AssetManager& assetManager);
    Ref<Mesh> ProcessMesh(aiMesh* mesh, AssetManager& assetManager);

private:
    std::vector<Ref<Mesh>> m_meshes;
    std::vector<Ref<Material>> m_materials;
    
    std::filesystem::path m_directory;
    Ref<Shader> m_defaultShader;

// Construction must be called by the AssetManager class through the Create method
friend class AssetManager;
protected:
    Model(const std::filesystem::path& filePath, Ref<Shader> defaultShader, AssetManager& assetManager);
    static Ref<Model> Create(const std::filesystem::path& filePath, Ref<Shader> defaultShader, AssetManager& assetManager)
    {
        return std::shared_ptr<Model>(new Model(filePath, defaultShader, assetManager));
    }
};


} /* namespace blr::core */