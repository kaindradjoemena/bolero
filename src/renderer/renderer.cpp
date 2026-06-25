// renderer/renderer.cpp

#include "renderer.hpp"
#include "core/camera.hpp"
#include "core/shader.hpp"
#include "core/material.hpp"
#include "core/mesh.hpp"
#include "core/wrappers/buffer.hpp"
#include "core/wrappers/vertex_array.hpp"
#include "core/wrappers/buffer.hpp"
#include "core/lights.hpp"
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
    s_cameraUBO->Bind(BufferLoc::CAM_UBO);

    s_instanceSSBO = ShaderStorageBuffer::Create();
    s_instanceSSBO->Bind(BufferLoc::INSTANCE_SSBO);

    s_lightSSBO = ShaderStorageBuffer::Create();
    s_lightSSBO->Bind(BufferLoc::LIGHT_SSBO);

    s_emptyVAO = VertexArray::Create();
}

void Renderer::Shutdown()
{
    s_opaqueQueue.clear();
    s_transparentQueue.clear();
    s_shadowCasterQueue.clear();

    s_instanceBuffer.clear();
}

void Renderer::BeginFrame()
{
    s_opaqueQueue.clear();
    s_transparentQueue.clear();
    s_shadowCasterQueue.clear();

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
    lightBuffer.dirCount   = static_cast<uint32_t>(std::min(int(s_dirLightBuffer.size()),   MAX_DIR_LIGHTS));
    lightBuffer.spotCount  = static_cast<uint32_t>(std::min(int(s_spotLightBuffer.size()),  MAX_SPOT_LIGHTS));
    lightBuffer.pointCount = static_cast<uint32_t>(std::min(int(s_pointLightBuffer.size()), MAX_POINT_LIGHTS));
    lightBuffer.padding    = 0;

    std::copy_n(s_dirLightBuffer.begin(),   lightBuffer.dirCount,   lightBuffer.dirLights);
    std::copy_n(s_pointLightBuffer.begin(), lightBuffer.pointCount, lightBuffer.pointLights);
    std::copy_n(s_spotLightBuffer.begin(),  lightBuffer.spotCount,  lightBuffer.spotLights);

    s_lightSSBO->SetData(&lightBuffer, sizeof(GPULightBuffer));
}

void Renderer::Submit(const Ref<Mesh>& mesh, const Ref<Material>& material, const Transform& transform, bool castsShadow)
{
    mat4 normalMat = Transpose(Inverse(transform.GetModelMat()));

    uint32_t transformIndex = static_cast<uint32_t>(s_instanceBuffer.size());

    s_instanceBuffer.push_back({ transform.GetModelMat(), normalMat });

    uint64_t depthBits    = 0;
    uint64_t shaderBits   = (material->GetShader()->GetHandle() & 0xFFFF);
    uint64_t materialBits = (material->GetHandle() & 0xFFFF);
    uint64_t meshBits     = (mesh->GetHandle() & 0xFFFF);

    uint64_t sortKey = (depthBits << 48) | (shaderBits << 32) | (materialBits << 16) | meshBits;

    RenderTask renderTask = { sortKey, mesh.get(), material.get(), transformIndex };


    if (castsShadow)
        s_shadowCasterQueue.emplace_back(renderTask);

    s_opaqueQueue.emplace_back(renderTask);
}

void Renderer::Submit(const DirLight& l, int shadowIndex)
{
    s_dirLightBuffer.push_back({
        vec4(l.direction, l.base.power),
        vec4(l.base.color, static_cast<float>(shadowIndex))});
}

void Renderer::Submit(const PointLight& l, int shadowIndex)
{
    s_pointLightBuffer.push_back({
        vec4(l.position, l.range),
        vec4(l.base.color, l.base.power),
        vec4(static_cast<float>(shadowIndex), 0.0f, 0.0f, 0.0f)});
}

void Renderer::Submit(const SpotLight& l, int shadowIndex)
{
    s_spotLightBuffer.push_back({
        vec4(l.position, l.length),
        vec4(l.direction, l.innerCos),
        vec4(l.base.color, l.outerCos),
        vec4(l.base.power, static_cast<float>(shadowIndex), 0.0f, 0.0f)});
}

void Renderer::DrawQueue(RenderQueueType queueType, Shader* overrideShader)
{
    std::vector<RenderTask>* targetQueue = &s_opaqueQueue;
    if (queueType == RenderQueueType::SHADOW_CASTER)
    {
        targetQueue = &s_shadowCasterQueue;
    }
    else if (queueType == RenderQueueType::TRANSPARENT)
    {
        targetQueue = &s_transparentQueue;
    }

    if (targetQueue->empty())
        return;

    // Sort requested queue
    std::sort(targetQueue->begin(), targetQueue->end());

    if (overrideShader)
        overrideShader->Bind();
    
    uint64_t lastShaderID   = 0;
    uint64_t lastMaterialID = 0;
    uint64_t lastMeshID     = 0;
    for (const RenderTask& task : *targetQueue)
    {
        if (!overrideShader)
        {
            uint64_t currentShaderID = task.material->GetShader()->GetHandle();
            if (currentShaderID != lastShaderID)
            {
                task.material->GetShader()->Bind();
                lastShaderID = currentShaderID;
            }
        }

        if (task.material->GetHandle() != lastMaterialID)
        {
            if (overrideShader)
                overrideShader->SetSuppressWarnings(true);

            task.material->Bind(overrideShader);

            if (overrideShader)
                overrideShader->SetSuppressWarnings(false);

            lastMaterialID = task.material->GetHandle();
        }

        if (!overrideShader)
        {
            task.material->GetShader()->SetUInt("u_TransformIndex", task.transformIndex);
        }
        else
        {
            overrideShader->SetUInt("u_TransformIndex", task.transformIndex);
        }

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