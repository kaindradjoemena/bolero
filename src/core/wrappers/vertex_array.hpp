// core/wrappers/vertex_array.hpp

#pragma once

#include <glad/glad.h>

#include "utils/base.hpp"

#include <vector>


namespace blr::core
{


class VertexBuffer;
class IndexBuffer;

class VertexArray
{
public:
    static Ref<VertexArray> Create()
    {
        return std::shared_ptr<VertexArray>(new VertexArray());
    }

    ~VertexArray();

    // Prevent copying
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    // Allow moving
    VertexArray(VertexArray&& other) = default;
    VertexArray& operator=(VertexArray&& other) = default;


    void Bind() const;
    void Unbind() const;

    // std::shared_ptr --> multiple meshes might share the same VB/IB
    void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vBuffer);
    void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& iBuffer);

    const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const { return m_vertexBuffers; }
    const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const { return m_indexBuffer; }
    GLuint GetRendererID() const { return m_rendererID; }

private:
    VertexArray();

    GLuint m_rendererID{0};
    uint32_t m_vertexBufferIndex = 0;

    std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
    std::shared_ptr<IndexBuffer> m_indexBuffer;
};


} /* namespace blr::core */