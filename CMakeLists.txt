cmake_minimum_required(VERSION 3.20)
project(Bolero VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BOLERO_BUILD_SHARED "Build shared libraries" OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
endforeach()

# Dependencies
include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

# GLAD check
set(GLAD_C_SOURCE ${CMAKE_SOURCE_DIR}/extern/glad/src/glad.c)
set(GLAD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/glad/include)

if(NOT EXISTS ${GLAD_C_SOURCE})
    message(FATAL_ERROR
        "GLAD not found at ${GLAD_C_SOURCE}.\n"
        "Generate GLAD (https://glad.dav1d.de) and place it in extern/glad/"
    )
endif()

# Source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
)

list(APPEND SOURCES ${GLAD_C_SOURCE})

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/extern
    ${GLAD_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/extern/stb
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glm::glm
)

# Platform specifics
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BOLERO_PLATFORM_WINDOWS)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)
elseif(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BOLERO_PLATFORM_MACOS)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVID_LIBRARY CoreVideo)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVID_LIBRARY})
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BOLERO_PLATFORM_LINUX)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL ${CMAKE_DL_LIBS})
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy assets on build
if(EXISTS ${CMAKE_SOURCE_DIR}/assets)
    add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
        COMMENT "Copying assets to build directory"
    )
    add_dependencies(${PROJECT_NAME} copy_assets)
else()
    message(STATUS "No assets directory found, skipping copy")
endif()