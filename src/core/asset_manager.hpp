// core/asset_manager.hpp

#pragma once

#include "utils/base.hpp"
#include "texture.hpp"

#include <filesystem>
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>


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

struct ShaderCacheEntry
{
    Ref<Shader> shader;
    std::filesystem::file_time_type lastWrite;
};

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


    Ref<Shader> CreateShader(std::string_view filePath);

    Ref<Tex> CreateTex(std::string_view texPath, const TexSpec& texSpec = TexSpec{});

    Ref<Tex> CreateTex(const TexSpec& texSpec);

    Ref<Material> CreateMaterial(const Ref<Shader>& shader);

    Ref<Mesh> CreateMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, const Ref<Material>& material);

    Ref<Model> CreateModel(std::string_view filePath, const Ref<Shader>& defaultShader);

    void Update();

private:
    std::unordered_map<std::string, ShaderCacheEntry> m_shaderCache;
    std::unordered_map<std::string, Ref<Tex>> m_texCache;
};


} /* namespace blr::core */