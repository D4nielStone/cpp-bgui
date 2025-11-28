cmake_minimum_required(VERSION 3.16)
project(bgui VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BGUI_USE_OPENGL     "Enable OpenGL backend"     ON)
option(BGUI_USE_GLFW       "Enable GLFW backend"       ON)
option(BGUI_USE_FREETYPE   "Enable FreeType support"   ON)
option(BGUI_BUILD_EXEMPLES "Build examples"            ON)

add_subdirectory(core)
add_subdirectory(backend)

add_library(bgui INTERFACE)
target_link_libraries(bgui
    INTERFACE
        bgui_core
        bgui_backend
)

target_include_directories(bgui
    INTERFACE
        ${PROJECT_SOURCE_DIR}/core/include
)

if(BGUI_BUILD_EXEMPLES)
    add_subdirectory(exemples)
endif()