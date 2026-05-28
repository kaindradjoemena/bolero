// utils/debug.hpp

#pragma once

#include <glad/glad.h>

#include <iostream>


// Debug Flags
#define DEBUG_RESOURCE_CREATION_HANDLE 1


#ifndef NDEBUG

inline
void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            GLuint id, 
                            GLenum severity, 
                            GLsizei length, 
                            const GLchar* message, 
                            const void* userParam)
{
    // Ignore performance/info from drivers
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return; 

    auto const src_str = [source]() {
        switch (source)
        {
            case GL_DEBUG_SOURCE_API:             return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "WINDOW SYSTEM";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
            case GL_DEBUG_SOURCE_THIRD_PARTY:     return "THIRD PARTY";
            case GL_DEBUG_SOURCE_APPLICATION:     return "APPLICATION";
            case GL_DEBUG_SOURCE_OTHER:           return "OTHER";
            default:                              return "UNKNOWN";
        }}();

    auto const type_str = [type]() {
        switch (type)
        {
            case GL_DEBUG_TYPE_ERROR:               return "ERROR";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "UNDEFINED_BEHAVIOR";
            case GL_DEBUG_TYPE_PORTABILITY:         return "PORTABILITY";
            case GL_DEBUG_TYPE_PERFORMANCE:         return "PERFORMANCE";
            case GL_DEBUG_TYPE_MARKER:              return "MARKER";
            case GL_DEBUG_TYPE_OTHER:               return "OTHER";
            default:                                return "UNKNOWN";
        }}();

    auto const severity_str = [severity]() {
        switch (severity)
        {
            case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
            case GL_DEBUG_SEVERITY_LOW:          return "LOW";
            case GL_DEBUG_SEVERITY_MEDIUM:       return "MEDIUM";
            case GL_DEBUG_SEVERITY_HIGH:         return "HIGH";
            default:                             return "UNKNOWN";
        }}();

    std::cout << "[" << severity_str << " | " << src_str << " | " << type_str << "] (" << id << "): " << message << '\n';

    // NOTE: Placing a breakpoint here should be help with tracing errors(?)
    if (severity == GL_DEBUG_SEVERITY_HIGH)
    {
        std::cerr << "Fatal OpenGL Error! Check callstack" << std::endl;
    }
}
#endif


namespace blr::utils
{


void PrintSystemInfo();


} /* namespace blr::utils */