// core/buffer.h

#pragma once

#include "../types.hpp"

#include <vector>


namespace blr::core
{


struct BufferElement
{
    std::string name;
    ShaderDataType type;
    uint32_t size;
    uint32_t offset;
    bool normalized;

    BufferElement(ShaderDataType t, const std::string& n, bool norm = false)
        : name(n), type(t), size(ShaderDataTypeSize(t)), offset(0), normalized(norm) {}
};


class BufferLayout
{
public:
    BufferLayout() = default;
    BufferLayout(std::initializer_list<BufferElement> e)
        : m_elements(e)
    {
        CalculateOffsetsAndStride();
    }

    inline uint32_t GetStride() const { return m_stride; }
    inline const std::vector<BufferElement>& GetElements() const { return m_elements; }

    std::vector<BufferElement>::iterator       begin() { return m_elements.begin(); }
    std::vector<BufferElement>::iterator       end()   { return m_elements.end(); }
    std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }
    std::vector<BufferElement>::const_iterator end() const   { return m_elements.end(); }

private:
    std::vector<BufferElement> m_elements;
    uint32_t m_stride = 0;

    void CalculateOffsetsAndStride()
    {
        uint32_t offset = 0;
        m_stride = 0;

        for (auto& element : m_elements)
        {
            element.offset = offset;
            offset   += element.size;
            m_stride += element.size;
        }
    }
};


class VertexBuffer
{
public:
    static Ref<VertexBuffer> Create(const void* verts, uint32_t size)
    {
        return std::shared_ptr<VertexBuffer>(new VertexBuffer(verts, size));
    }

    ~VertexBuffer();

    // Prevent copying
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    // Allow moving
    VertexBuffer(VertexBuffer&& other) = default;
    VertexBuffer& operator=(VertexBuffer&& other) = default;


    void Bind() const;
    void Unbind() const;

    const BufferLayout& GetLayout() const { return m_layout; }
    void SetLayout(const BufferLayout& layout) { m_layout = layout; }
    GLuint GetRendererID() const { return m_rendererID; }

private:
    VertexBuffer(const void* verts, uint32_t size);

    GLuint m_rendererID{0};
    BufferLayout m_layout;
};


class IndexBuffer
{
public:
    static Ref<IndexBuffer> Create(const uint32_t* indices, uint32_t count)
    {
        return std::shared_ptr<IndexBuffer>(new IndexBuffer(indices, count));
    }

    ~IndexBuffer();

    // Prevent copying
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    // Allow moving
    IndexBuffer(IndexBuffer&& other) = default;
    IndexBuffer& operator=(IndexBuffer&& other) = default;


    void Bind() const;
    void Unbind() const;

    uint32_t GetCount() const { return m_count; }
    GLuint GetRendererID() const { return m_rendererID; }

private:
    IndexBuffer(const uint32_t* indices, uint32_t count);

    GLuint m_rendererID{0};
    uint32_t m_count{0};
};


} /* namespace blr::core */