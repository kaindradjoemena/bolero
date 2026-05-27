// core/asset_manager.hpp

#pragma once

#include "types.hpp"

#include <filesystem>


namespace blr::core
{


class Shader;
class VertexBuffer;
class IndexBuffer;
class VertexArray;
class Material;
class Vertex;
class Tex;
class Mesh;
class Model;

class AssetManager
{
public:
    AssetManager() = default;
    ~AssetManager() = default;

    // Prevent copying
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // Prevent moving
    AssetManager(AssetManager&& other) = delete;
    AssetManager& operator=(AssetManager&& other) = delete;


    Ref<Shader> CreateShader(const std::filesystem::path& filePath);

    Ref<Tex> CreateTex(const std::filesystem::path& texPath, const TexSpec& texSpec = TexSpec{});

    Ref<Tex> CreateTex(const TexSpec& texSpec);

    Ref<Material> CreateMaterial(Ref<Shader> shader);

    Ref<Mesh> CreateMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Ref<Material> material);

    Ref<Model> CreateModel(const std::filesystem::path& filePath, Ref<Shader> defaultShader);
};


} /* namespace blr::core */