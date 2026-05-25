// core/asset_manager.cpp

#include "asset_manager.hpp"


namespace blr::core
{


Ref<Shader> AssetManager::CreateShader(const std::filesystem::path& filePath)
{
    Ref<Shader> shader = Shader::Create(filePath);

    return shader;
}

Ref<VertexBuffer> AssetManager::CreateVB(const void* verts, uint32_t size)
{
    Ref<VertexBuffer> vb = VertexBuffer::Create(verts, size);

    return vb;
}

Ref<IndexBuffer> AssetManager::CreateIB(const uint32_t* indices, uint32_t count)
{
    Ref<IndexBuffer> ib = IndexBuffer::Create(indices, count);

    return ib;
}

Ref<VertexArray> AssetManager::CreateVA()
{
    Ref<VertexArray> va = VertexArray::Create();

    return va;
}

Ref<Tex> AssetManager::CreateTex(const std::filesystem::path& texPath, const TexSpec& texSpec)
{
    Ref<Tex> tex = Tex::Create(texPath, texSpec);

    return tex;
}

Ref<Tex> AssetManager::CreateTex(const TexSpec& texSpec)
{
    Ref<Tex> tex = Tex::Create(texSpec);

    return tex;
}

Ref<Material> AssetManager::CreateMaterial(Ref<Shader> shader)
{
    Ref<Material> mat = Material::Create(shader);

    return mat;
}

Ref<Mesh> AssetManager::CreateMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Ref<Material> material)
{
    Ref<Mesh> mesh = Mesh::Create(vertices, indices, material, *this);

    return mesh;
}

Ref<Model> AssetManager::CreateModel(const std::filesystem::path& filePath, Ref<Shader> defaultShader)
{
    Ref<Model> model = Model::Create(filePath, defaultShader, *this);

    return model;
}


} /* namespace blr::core */