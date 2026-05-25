// core/asset_manager.hpp

#pragma once

#include "types.hpp"
#include "buffer.hpp"
#include "vertex_array.hpp"
#include "texture.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "utils/uuid.hpp"


namespace blr::core
{


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

    Ref<VertexBuffer> CreateVB(const void* verts, uint32_t size);

    Ref<IndexBuffer> CreateIB(const uint32_t* indices, uint32_t count);

    Ref<VertexArray> CreateVA();

    Ref<Tex> CreateTex(const std::filesystem::path& texPath, const TexSpec& texSpec = TexSpec{});

    Ref<Tex> CreateTex(const TexSpec& texSpec);

    Ref<Material> CreateMaterial(Ref<Shader> shader);

    Ref<Mesh> CreateMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Ref<Material> material);

    Ref<Model> CreateModel(const std::filesystem::path& filePath, Ref<Shader> defaultShader);
};


} /* namespace blr::core */