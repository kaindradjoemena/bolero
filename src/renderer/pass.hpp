// renderer/pass.hpp

#pragma once

#include "core/types.hpp"
#include <string>


namespace blr::core
{


class Scene;

class RenderPass
{
public:
    RenderPass(const std::string& name) : m_name(name) {}
    virtual ~RenderPass() = default;

    // Called once when added to the pipeline by RenderPipeline::AddPass()
    virtual void Init() = 0;

    // This is where you configure GL state and issue Renderer draw commands.
    // Called every frame.
    virtual void Execute(Scene& scene) = 0;

    // Called when done
    virtual void Shutdown() = 0;

    const std::string& GetName() const { return m_name; }

protected:
    std::string m_name;
};


} /* namespace blr::core */