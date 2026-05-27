// renderer/renderer.cpp

#include "renderer.hpp"
#include "core/shader.hpp"
#include "core/material.hpp"
#include "core/mesh.hpp"
#include "core/wrappers/buffer.hpp"
#include "core/wrappers/vertex_array.hpp"
#include "utils/debug.hpp"

#include <algorithm>


namespace blr::core
{


void Renderer::Init()
{
#ifndef NDEBUG
    int flags; 
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        std::cout << "OpenGL Debug Context initialized\n";
    }
#endif

    glCreateBuffers(1, &s_SSBO);
    glCreateBuffers(1, &s_lightSSBO);
}

void Renderer::Shutdown()
{
    glDeleteBuffers(1, &s_SSBO);
    glDeleteBuffers(1, &s_lightSSBO);
}

void Renderer::BeginScene(const Camera& camera)
{
    s_renderQueue.clear();
    s_instanceBuffer.clear();

    s_dirLightBuffer.clear();
    s_pointLightBuffer.clear();
    s_spotLightBuffer.clear();
}

void Renderer::Submit(const Ref<Mesh>& mesh, const Ref<Material>& material, const Transform& transform)
{
    mat4 normalMat = Transpose(Inverse(transform.GetModelMat()));

    uint32_t transformIndex = static_cast<uint32_t>(s_instanceBuffer.size());

    s_instanceBuffer.push_back({ transform.GetModelMat(), normalMat });

    uint64_t depthBits    = 0;
    uint64_t shaderBits   = (material->GetShader()->GetHandle() & 0xFFFF);
    uint64_t materialBits = (material->GetHandle() & 0xFFFF);
    uint64_t meshBits     = (mesh->GetHandle() & 0xFFFF);

    uint64_t sortKey = (depthBits << 48) | (shaderBits << 32) | (materialBits << 16) | meshBits;

    s_renderQueue.push_back({
        sortKey,
        mesh.get(),
        material.get(),
        transformIndex});

}

void Renderer::Submit(const DirLight& l)
{
    s_dirLightBuffer.push_back({
        vec4(l.direction, l.base.power),
        vec4(l.base.color, 1.0f)});
}

void Renderer::Submit(const PointLight& l)
{
    s_pointLightBuffer.push_back({
        vec4(l.position, l.range),
        vec4(l.base.color, l.base.power)});
}

void Renderer::Submit(const SpotLight& l)
{
    s_spotLightBuffer.push_back({
        vec4(l.position, l.length),
        vec4(l.direction, l.innerCos),
        vec4(l.base.color, l.outerCos),
        vec4(l.base.power, vec3(1.0f))});
}


void Renderer::Render()
{
    if (s_renderQueue.empty())
        return;

    std::sort(s_renderQueue.begin(), s_renderQueue.end());

    // Upload instace matrices
    glNamedBufferData(s_SSBO, s_instanceBuffer.size() * sizeof(InstanceData), s_instanceBuffer.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, s_SSBO);

    // Upload light data
    GPULightBuffer lightBuffer;
    lightBuffer.dirCount   = static_cast<uint32_t>(s_dirLightBuffer.size());
    lightBuffer.pointCount = static_cast<uint32_t>(s_pointLightBuffer.size());
    lightBuffer.spotCount  = static_cast<uint32_t>(s_spotLightBuffer.size());

    std::copy_n(s_dirLightBuffer.begin(),   std::min(lightBuffer.dirCount,   16u),    lightBuffer.dirLights);
    std::copy_n(s_pointLightBuffer.begin(), std::min(lightBuffer.pointCount, 1024u), lightBuffer.pointLights);
    std::copy_n(s_spotLightBuffer.begin(),  std::min(lightBuffer.spotCount,  512u),   lightBuffer.spotLights);

    glNamedBufferData(s_lightSSBO, sizeof(GPULightBuffer), &lightBuffer, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, s_lightSSBO);

    uint64_t lastMaterialID = 0;
    uint64_t lastMeshID = 0;
    for (const RenderTask& task : s_renderQueue)
    {
        if (task.material->GetHandle() != lastMaterialID)
        {
            task.material->Bind();
            lastMaterialID = task.material->GetHandle();
        }

        task.material->GetShader()->SetUInt("u_TransformIndex", task.transformIndex);

        if (task.mesh->GetHandle() != lastMeshID)
        {
            task.mesh->GetVAO()->Bind();
            lastMeshID = task.mesh->GetHandle();
        }

        glDrawElements(GL_TRIANGLES, task.mesh->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);
    }
}


} /* namespace blr::core */