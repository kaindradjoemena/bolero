// renderer/renderer.hpp

#pragma once

#include "utils/base.hpp"
#include "utils/math.hpp"
#include "core/transform.hpp"
#include "core/gpu_layout.hpp"

#include <vector>
#include <cstdint>


namespace blr::core
{


class Mesh;
class Material;
class Camera;
class Shader;
class UniformBuffer;
class VertexArray;
class ShaderStorageBuffer;
struct DirLight;
struct PointLight;
struct SpotLight;

enum class RenderQueueType
{
    OPAQUE,
    TRANSPARENT,
    SHADOW_CASTER
};

struct RenderTask
{
    uint64_t sortKey;
    Mesh* mesh;
    Material* material;
    uint32_t transformIndex;

    bool operator<(const RenderTask& other) const
    {
        return sortKey < other.sortKey;
    }
};

struct RenderStats
{
    uint32_t drawCalls = 0;
    
    void Reset()
    {
        drawCalls = 0;
    }
};


class Renderer
{
public:
    Renderer() = delete;
    ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    Renderer(Renderer&& other) = delete;
    Renderer& operator=(Renderer&& other) = delete;

    static void SetRenderScale(float scale) { s_renderScale = Clamp(scale, 0.1f, 4.0f); }  // 10% - 400% native
    static void SetViewportResolution(uint32_t w, uint32_t h) { s_viewportWidth = w; s_viewportHeight = h; }
    
    static float GetRenderScale() { return s_renderScale; }
    static uint32_t GetViewportWidth() { return s_viewportWidth; }
    static uint32_t GetViewportHeight() { return s_viewportHeight; }

    static const RenderStats& GetRenderStats() { return s_stats; }

    static void Init();
    static void Shutdown();

    static void BeginFrame();

    static void UpdateCameraUBO(const Camera& camera);
    static void UpdateCameraUBO(const mat4& view, const mat4& proj, const vec3& pos);
    static void UploadBuffers(); 
    
    static void Submit(const Ref<Mesh>& mesh, const Ref<Material>& material, const Transform& transform, bool castsShadow = true);
    static void Submit(const DirLight& l, int shadowIndex);
    static void Submit(const PointLight& l, int shadowIndex);
    static void Submit(const SpotLight& l, int shadowIndex);

    static void DrawQueue(RenderQueueType queueType, Shader* overrideShader = nullptr);
    static void DrawFullscreenQuad();
    static void DrawCube();

private:
    inline static float s_renderScale     = 1.0f;
    inline static uint32_t s_viewportWidth  = 512;
    inline static uint32_t s_viewportHeight = 512;

    inline static std::vector<RenderTask> s_opaqueQueue;
    inline static std::vector<RenderTask> s_transparentQueue;
    inline static std::vector<RenderTask> s_shadowCasterQueue;

    inline static std::vector<InstanceData> s_instanceBuffer;

    inline static std::vector<DirLightData>   s_dirLightBuffer;
    inline static std::vector<PointLightData> s_pointLightBuffer;
    inline static std::vector<SpotLightData>  s_spotLightBuffer;

    inline static Ref<UniformBuffer>       s_cameraUBO;
    inline static Ref<ShaderStorageBuffer> s_instanceSSBO;
    inline static Ref<ShaderStorageBuffer> s_lightSSBO;
    inline static Ref<VertexArray>         s_emptyVAO;

    inline static RenderStats s_stats;
};


} /* namespace blr::core */