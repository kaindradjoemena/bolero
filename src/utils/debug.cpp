// utils/debug.cpp

#include "debug.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <assimp/version.h>


namespace blr::utils
{


void PrintSystemInfo()
{
    std::cout << "GPU:            " << glGetString(GL_RENDERER) << " (" << glGetString(GL_VENDOR) << ")\n";
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GLFW Version:   " << glfwGetVersionString() << "\n";
    std::cout << "GLM Version:    " << GLM_VERSION_MAJOR << "." << GLM_VERSION_MINOR << "." << GLM_VERSION_PATCH << "\n";
    std::cout << "Assimp Version: " << aiGetVersionMajor() << "." << aiGetVersionMinor() << "." << aiGetVersionPatch() << "\n";
}


} /* namespace blr::utils */