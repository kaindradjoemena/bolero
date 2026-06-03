// renderer/renderer.cpp

#include "renderer.hpp"
#include "core/camera.hpp"
#include "core/shader.hpp"
#include "core/material.hpp"
#include "core/mesh.hpp"
#include "core/wrappers/buffer.hpp"
#include "core/wrappers/vertex_array.hpp"
#include "core/wrappers/buffer.hpp"
#include "utils/debug.hpp"

#include <algorithm>

#include <glfw/glfw3.h>


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

    s_cameraUBO = UniformBuffer::Create(sizeof(CameraFrameData));
    s_cameraUBO->Bind(0);

    s_instanceSSBO = ShaderStorageBuffer::Create();
    s_instanceSSBO->Bind(1);

    s_lightSSBO = ShaderStorageBuffer::Create();
    s_lightSSBO->Bind(2);

    s_emptyVAO = VertexArray::Create();
}

void Renderer::Shutdown()
{
    s_renderQueue.clear();
    s_instanceBuffer.clear();
}

void Renderer::BeginFrame()
{
    s_renderQueue.clear();
    s_instanceBuffer.clear();
    s_dirLightBuffer.clear();
    s_pointLightBuffer.clear();
    s_spotLightBuffer.clear();

    s_stats.Reset();
}

void Renderer::UpdateCameraUBO(const Camera& camera)
{
    Renderer::UpdateCameraUBO(camera.GetViewMat(), camera.GetProjMat(), camera.GetPos());
}

void Renderer::UpdateCameraUBO(const mat4& view, const mat4& proj, const vec3& pos)
{
    CameraFrameData camData;
    camData.view             = view;
    camData.projection       = proj;
    camData.viewProj         = camData.projection * camData.view;
    camData.cameraPosAndTime = vec4(pos, (float)glfwGetTime());

    s_cameraUBO->SetData(&camData, sizeof(CameraFrameData), 0);
}

void Renderer::UploadBuffers()
{
    // Instance data
    if (!s_instanceBuffer.empty())
        s_instanceSSBO->SetData(s_instanceBuffer.data(), s_instanceBuffer.size() * sizeof(InstanceData));

    // Light data
    GPULightBuffer lightBuffer;
    lightBuffer.dirCount   = static_cast<uint32_t>(std::min(s_dirLightBuffer.size(),   16ull));
    lightBuffer.pointCount = static_cast<uint32_t>(std::min(s_pointLightBuffer.size(), 1024ull));
    lightBuffer.spotCount  = static_cast<uint32_t>(std::min(s_spotLightBuffer.size(),  512ull));
    lightBuffer.padding    = 0;

    std::copy_n(s_dirLightBuffer.begin(),   lightBuffer.dirCount,   lightBuffer.dirLights);
    std::copy_n(s_pointLightBuffer.begin(), lightBuffer.pointCount, lightBuffer.pointLights);
    std::copy_n(s_spotLightBuffer.begin(),  lightBuffer.spotCount,  lightBuffer.spotLights);

    s_lightSSBO->SetData(&lightBuffer, sizeof(GPULightBuffer));
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

void Renderer::DrawQueue(Shader* overrideShader)
{
    if (s_renderQueue.empty())
        return;

    // Sort queue
    std::sort(s_renderQueue.begin(), s_renderQueue.end());

    if (overrideShader)
        overrideShader->Bind();
    
    uint64_t lastMaterialID = 0;
    uint64_t lastMeshID = 0;
    for (const RenderTask& task : s_renderQueue)
    {
        if (!overrideShader)
        {
            if (task.material->GetHandle() != lastMaterialID)
            {
                task.material->Bind();
                lastMaterialID = task.material->GetHandle();
            }

            task.material->GetShader()->SetUInt("u_TransformIndex", task.transformIndex);
        }
        else
        {
            overrideShader->SetUInt("u_TransformIndex", task.transformIndex);
        }

        // Bind mesh geometry
        if (task.mesh->GetHandle() != lastMeshID)
        {
            task.mesh->GetVAO()->Bind();
            lastMeshID = task.mesh->GetHandle();
        }

        glDrawElements(GL_TRIANGLES, task.mesh->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);
    
        s_stats.drawCalls++;
    }
}

void Renderer::DrawFullscreenQuad()
{
    s_emptyVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);   // make the triangle in the vertex shader

    s_stats.drawCalls++;
}

void Renderer::DrawCube()
{
    s_emptyVAO->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);   

    s_stats.drawCalls++;
}


} /* namespace blr::core */