// core/buffer.cpp

#include "buffer.hpp"

#include <utility>


namespace blr::core
{


// ===== Class VertexBuffer =====
VertexBuffer::VertexBuffer(const void* verts, uint32_t size)
{
    glCreateBuffers(1, &m_rendererID);
    glNamedBufferStorage(m_rendererID, size, verts, GL_DYNAMIC_STORAGE_BIT);
}

VertexBuffer::~VertexBuffer()
{
    if (m_rendererID != 0)
        glDeleteBuffers(1, &m_rendererID);
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
}

void VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// ===== Class IndexBuffer =====
IndexBuffer::IndexBuffer(const uint32_t* indices, uint32_t count)
    : m_count(count)
{
    glCreateBuffers(1, &m_rendererID);
    glNamedBufferStorage(m_rendererID, count * sizeof(uint32_t), indices, GL_DYNAMIC_STORAGE_BIT);
}

IndexBuffer::~IndexBuffer()
{
    if (m_rendererID != 0)
        glDeleteBuffers(1, &m_rendererID);
}

void IndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
}

void IndexBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


} /* namespace blr::core */