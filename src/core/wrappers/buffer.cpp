// core/wrappers/buffer.cpp

#include "buffer.hpp"


namespace blr::core
{


// ===== VertexBuffer =====
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


// ===== IndexBuffer =====
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


// ===== UniformBuffer =====
UniformBuffer::UniformBuffer(uint32_t size)
{
    glCreateBuffers(1, &m_rendererID);
    glNamedBufferData(m_rendererID, size, nullptr, GL_DYNAMIC_DRAW); 
}

UniformBuffer::~UniformBuffer()
{
    if (m_rendererID != 0)
        glDeleteBuffers(1, &m_rendererID);
}

void UniformBuffer::Bind(uint32_t binding) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_rendererID);
}

void UniformBuffer::Unbind() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset) const
{
    glNamedBufferSubData(m_rendererID, offset, size, data);
}


// ===== ShaderStorageBuffer =====
ShaderStorageBuffer::ShaderStorageBuffer()
{
    glCreateBuffers(1, &m_rendererID);
}

ShaderStorageBuffer::~ShaderStorageBuffer()
{
    if (m_rendererID != 0)
        glDeleteBuffers(1, &m_rendererID);
}

void ShaderStorageBuffer::Bind(uint32_t binding) const
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_rendererID);
}

void ShaderStorageBuffer::Unbind() const
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ShaderStorageBuffer::SetData(const void* data, uint32_t size) const
{
    glNamedBufferData(m_rendererID, size, data, GL_DYNAMIC_DRAW);
}


} /* namespace blr::core */