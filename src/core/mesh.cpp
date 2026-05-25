// core/mesh.cpp

#include "mesh.hpp"
#include "asset_manager.hpp"

#include <utility>


namespace blr::core
{


Mesh::Mesh(std::vector<Vertex> vertices,
           std::vector<uint32_t> indices,
           Ref<Material> material,
           AssetManager& assetManager)
    : m_vertices(std::move(vertices))
    , m_indices(std::move(indices))
    , m_material(std::move(material))
{
    SetupMesh(assetManager);
}

void Mesh::SetupMesh(AssetManager& assetManager)
{
    if (m_vertices.empty() || m_indices.empty())
    {
        std::cout << "[Warning] Skipped empty mesh during setup." << std::endl;
        return; 
    }

    m_vao = assetManager.CreateVA();

    uint32_t vertexSize = static_cast<uint32_t>(m_vertices.size() * sizeof(Vertex));
    m_vbo = assetManager.CreateVB(m_vertices.data(), vertexSize);    
    m_vbo->SetLayout({
        { ShaderDataType::Float3, "a_position"  },
        { ShaderDataType::Float3, "a_normal"    },
        { ShaderDataType::Float2, "a_texCoords" },
        { ShaderDataType::Float3, "a_tangent"   },
        { ShaderDataType::Float3, "a_bitangent" }
    });

    uint32_t indexCount = static_cast<uint32_t>(m_indices.size());
    m_ibo = assetManager.CreateIB(m_indices.data(), indexCount);
    
    m_vao->AddVertexBuffer(m_vbo);
    m_vao->SetIndexBuffer(m_ibo);
}


} /* namespace blr::core */