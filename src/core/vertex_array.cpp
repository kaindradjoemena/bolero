// core/vertex_array.cpp

#include "vertex_array.hpp"

#include <utility>


namespace BLR
{


// ===== Class VertexArray =====
VertexArray::VertexArray()
{
    glCreateVertexArrays(1, &m_rendererID);
}

VertexArray::~VertexArray()
{
    if (m_rendererID != 0)
        glDeleteVertexArrays(1, &m_rendererID);
}

VertexArray::VertexArray(VertexArray&& other) noexcept
    : m_rendererID(std::exchange(other.m_rendererID, 0))
    , m_vertexBufferIndex(std::exchange(other.m_vertexBufferIndex, 0))
    , m_vertexBuffers(std::move(other.m_vertexBuffers))
    , m_indexBuffer(std::move(other.m_indexBuffer)) {}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
    if (this != &other)
    {
        if (m_rendererID != 0)
            glDeleteVertexArrays(1, &m_rendererID);
        
        m_rendererID        = std::exchange(other.m_rendererID, 0);
        m_vertexBufferIndex = std::exchange(other.m_vertexBufferIndex, 0);
        m_vertexBuffers     = std::move(other.m_vertexBuffers);
        m_indexBuffer       = std::move(other.m_indexBuffer);
    }

    return *this;
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_rendererID);
}

void VertexArray::Unbind() const
{
    glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vBuffer)
{
    const BufferLayout& layout = vBuffer->GetLayout();

    // 1. Attach VB
    GLuint bindingIndex = m_vertexBuffers.size();   // the index where this specific VB gets attached to the VA
    glVertexArrayVertexBuffer(m_rendererID, bindingIndex, vBuffer->GetRendererID(), 0, layout.GetStride());

    // 2. For every element (pos, col, uv, norm, etc.), bind it to the VA
    for (const auto& e : layout)
    {
        glEnableVertexArrayAttrib(m_rendererID, m_vertexBufferIndex);
        glVertexArrayAttribFormat(m_rendererID, m_vertexBufferIndex, 
                                  ShaderDataTypeComponentCount(e.type), 
                                  ShaderDataTypeToOpenGLBaseType(e.type), 
                                  e.normalized ? GL_TRUE : GL_FALSE,
                                  e.offset);
        glVertexArrayAttribBinding(m_rendererID, m_vertexBufferIndex, bindingIndex);

        m_vertexBufferIndex++;
    }

    m_vertexBuffers.emplace_back(vBuffer);
}

void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& iBuffer)
{
    glVertexArrayElementBuffer(m_rendererID, iBuffer->GetRendererID());
    m_indexBuffer = iBuffer;
}

} /* namespace BLR */