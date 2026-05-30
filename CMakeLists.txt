cmake_minimum_required(VERSION 3.20)
project(Bolero VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


option(BOLERO_BUILD_SHARED  "Build Bolero as a shared library" OFF)
option(BOLERO_BUILD_SANDBOX "Build the sandbox executable"     ON)


if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(BOLERO_BUILD_SANDBOX OFF)
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


find_package(glfw3  QUIET)
find_package(glm    QUIET)
find_package(assimp QUIET)

include(FetchContent)

if(NOT glfw3_FOUND)
    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
        GIT_SHALLOW    TRUE)
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

if(NOT glm_FOUND)
    FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.3
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(glm)
endif()

if(NOT assimp_FOUND)
    FetchContent_Declare(assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG        v6.0.5
        GIT_SHALLOW    TRUE)
    
    set(ASSIMP_BUILD_TESTS                    OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ASSIMP_TOOLS             OFF CACHE BOOL "" FORCE)
    set(ASSIMP_INSTALL                        OFF CACHE BOOL "" FORCE)
    set(INJECT_DEBUG_POSTFIX                  OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_GLTF_IMPORTER            ON  CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_OBJ_IMPORTER             ON  CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_FBX_IMPORTER             ON  CACHE BOOL "" FORCE)
    set(ASSIMP_NO_EXPORT                      ON  CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ZLIB                     ON  CACHE BOOL "" FORCE)
    set(ASSIMP_WARNINGS_AS_ERRORS             OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS                     OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(assimp)
endif()


set(GLAD_C_SOURCE    ${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/src/glad.c)
set(GLAD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/include)

if(NOT EXISTS ${GLAD_C_SOURCE})
    message(FATAL_ERROR
        "GLAD not found at ${GLAD_C_SOURCE}.\n"
        "Generate GLAD (https://glad.dav1d.de) and place it in extern/glad/")
endif()


file(GLOB_RECURSE BOLERO_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)
list(APPEND BOLERO_SOURCES ${GLAD_C_SOURCE})

if(BOLERO_BUILD_SHARED)
    add_library(Bolero SHARED ${BOLERO_SOURCES})
else()
    add_library(Bolero STATIC ${BOLERO_SOURCES})
endif()

target_include_directories(Bolero PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${GLAD_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(Bolero PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/extern
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/stb
    $<$<NOT:$<BOOL:${assimp_FOUND}>>:${assimp_SOURCE_DIR}/include>
)

target_link_libraries(Bolero PUBLIC  glfw glm::glm)
target_link_libraries(Bolero PRIVATE assimp)


if(WIN32)
    target_compile_definitions(Bolero PUBLIC BOLERO_PLATFORM_WINDOWS)
    target_link_libraries(Bolero PRIVATE opengl32)

    if(MSVC)
        target_compile_options(Bolero PRIVATE
            /MP
            $<$<CONFIG:Debug>:/Od /Zi>
            $<$<CONFIG:Release>:/O2 /GL>
        )
        target_link_options(Bolero PRIVATE
            $<$<CONFIG:Debug>:/DEBUG:FASTLINK>
            $<$<CONFIG:Release>:/LTCG>
        )
    endif()
elseif(APPLE)
    target_compile_definitions(Bolero PUBLIC BOLERO_PLATFORM_MACOS)
    find_library(COCOA_LIBRARY   Cocoa   REQUIRED)
    find_library(IOKIT_LIBRARY   IOKit   REQUIRED)
    find_library(COREVID_LIBRARY CoreVideo REQUIRED)
    target_link_libraries(Bolero PRIVATE
        ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVID_LIBRARY})
elseif(UNIX)
    target_compile_definitions(Bolero PUBLIC BOLERO_PLATFORM_LINUX)
    find_package(OpenGL REQUIRED)
    target_link_libraries(Bolero PRIVATE OpenGL::GL ${CMAKE_DL_LIBS})
endif()


if(MSVC)
    target_compile_options(Bolero PRIVATE /W4 /permissive-)
else()
    target_compile_options(Bolero PRIVATE -Wall -Wextra -Wpedantic)
endif()


if(BOLERO_BUILD_SANDBOX)
    add_executable(BoleroSandbox
        "${CMAKE_CURRENT_SOURCE_DIR}/sandbox/main.cpp")

    target_compile_features(BoleroSandbox PRIVATE cxx_std_17)
    target_link_libraries(BoleroSandbox PRIVATE Bolero)

    target_compile_definitions(BoleroSandbox PRIVATE 
        SANDBOX_ROOT_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_target_properties(BoleroSandbox PROPERTIES 
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()