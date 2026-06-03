// bolero.hpp

#pragma once


// OpenGL & Math
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Application Layer
#include "app/window.hpp"
#include "app/input.hpp"
#include "utils/debug.hpp"

// Core Framework & State
#include "core/types.hpp"
#include "core/asset_manager.hpp"
#include "core/scene.hpp"
#include "core/camera.hpp"

// Renderer & Pass Architecture
#include "renderer/renderer.hpp"
#include "renderer/pass.hpp"
#include "renderer/pipeline.hpp"
#include "core/wrappers/framebuffer.hpp"

// Resources
#include "core/model.hpp"
#include "core/mesh.hpp"
#include "core/material.hpp"
#include "core/shader.hpp"
#include "core/lights.hpp"