// renderer/renderer.hpp

#pragma once

#include "core/types.hpp"
#include "core/lights.hpp"

#include <vector>
#include <cstdint>


namespace blr::core
{


class Mesh;
class Material;
class Camera;

struct DirLightData
{
    vec4 directionAndPower; // xyz = direction, w = power
    vec4 color;             // xyz = color, w = padding (1.0f)
};

struct PointLightData
{
    vec4 positionAndRange; // xyz = position, w = range
    vec4 colorAndPower;    // xyz = color, w = power
};

struct SpotLightData
{
    vec4 positionAndLength; // xyz = position, w = length
    vec4 directionAndInner; // xyz = direction, w = innerCos
    vec4 colorAndOuter;     // xyz = color, w = outerCos
    vec4 PowerAndPadding;   // x = power, yzw = padding (1.0f)
};

struct GPULightBuffer
{
    uint32_t dirCount{0};
    uint32_t pointCount{0};
    uint32_t spotCount{0};
    uint32_t padding{0};
    
    DirLightData dirLights[16];
    PointLightData pointLights[1024];
    SpotLightData spotLights[512];
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

struct InstanceData
{
    mat4 model;
    mat4 normal;
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


    static void Init();
    static void Shutdown();

    static void BeginScene(const Camera& camera);
    
    static void Submit(const Ref<Mesh>& mesh, const Ref<Material>& material, const Transform& transform);
    static void Submit(const DirLight& l);
    static void Submit(const PointLight& l);
    static void Submit(const SpotLight& l);

    static void Render();

private:
    inline static std::vector<RenderTask> s_renderQueue;
    inline static std::vector<InstanceData> s_instanceBuffer;

    inline static std::vector<DirLightData> s_dirLightBuffer;
    inline static std::vector<PointLightData> s_pointLightBuffer;
    inline static std::vector<SpotLightData> s_spotLightBuffer;

    inline static uint32_t s_SSBO = 0;
    inline static uint32_t s_lightSSBO = 0;
};


} /* namespace blr::core */