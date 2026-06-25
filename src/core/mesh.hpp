// core/mesh.hpp

#pragma once

#include "utils/base.hpp"
#include "utils/math.hpp"
#include "resource.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blr::core
{


class VertexBuffer;
class IndexBuffer;
class VertexArray;
class Material;
class AssetManager;

struct Vertex
{
    vec3 position;
    vec3 normal;
    vec2 texCoords;
    vec3 tangent;
    vec3 bitangent;
};


class Mesh : public Resource
{


public:
    ~Mesh() = default;

    // Prevent copying
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Allow moving
    Mesh(Mesh&& other) = default;
    Mesh& operator=(Mesh&& other) = default;


    Ref<VertexArray> GetVAO() const { return m_vao; }
    Ref<IndexBuffer> GetIBO() const { return m_ibo; }
    Ref<Material> GetMaterial() const { return m_material; }

    const std::vector<Vertex>& GetVertices() const { return m_vertices; }
    const std::vector<uint32_t>& GetIndices() const { return m_indices; }

private:
    void SetupMesh();

private:
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    Ref<Material> m_material;

    Ref<VertexArray> m_vao;
    Ref<VertexBuffer> m_vbo;
    Ref<IndexBuffer> m_ibo;

// Construction must be called by the AssetManager class through the Create method
friend class AssetManager;
protected:
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, const Ref<Material>& material);
    static Ref<Mesh> Create(std::vector<Vertex> vertices, std::vector<uint32_t> indices, const Ref<Material>& material)
    {
        return std::shared_ptr<Mesh>(new Mesh(vertices, indices, material));
    }
};


} /* namespace blr::core */