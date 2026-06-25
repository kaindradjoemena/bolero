// bolero.hpp

#pragma once


// OpenGL & Math
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Utils
#include "utils/base.hpp"
#include "utils/math.hpp"
#include "utils/debug.hpp"
#include "core/gpu_layout.hpp"

// Application Layer
#include "app/window.hpp"
#include "app/input.hpp"
#include "utils/debug.hpp"

// Core Framework & State
#include "core/image_format.hpp"
#include "core/image_buffer.hpp"
#include "core/frame_capturer.hpp"
#include "core/asset_manager.hpp"
#include "core/scene.hpp"
#include "core/camera.hpp"
#include "core/vfs.hpp"
#include "core/scene.hpp"
#include "core/texture.hpp"
#include "ui/ui.hpp"

// Renderer & Pass Architecture
#include "renderer/renderer.hpp"
#include "renderer/pass.hpp"
#include "renderer/pipeline.hpp"
#include "core/wrappers/framebuffer.hpp"
#include "core/render_context.hpp"

// Resources
#include "core/model.hpp"
#include "core/mesh.hpp"
#include "core/material.hpp"
#include "core/shader.hpp"
#include "core/lights.hpp"
#include "core/wrappers/cubemap.hpp"

#include "ui/ui.hpp"


#include "core/passes.hpp"