cmake_minimum_required(VERSION 3.16)
project(bgui VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
cmake_minimum_required(VERSION 3.15)
project(BGUI)

# --- Backend options ---
option(BGUI_USE_OPENGL   "Enable OpenGL backend"   OFF)
option(BGUI_USE_VULKAN   "Enable Vulkan backend"   OFF)
option(BGUI_USE_GLFW     "Enable GLFW backend"     ON)
option(BGUI_USE_FREETYPE "Enable FreeType support" ON)
option(BGUI_BUILD_EXAMPLES "Build examples"        ON)

# --- User Message ---
message(STATUS "OpenGL backend:   ${BGUI_USE_OPENGL}")
message(STATUS "Vulkan backend:   ${BGUI_USE_VULKAN}")
message(STATUS "GLFW backend:     ${BGUI_USE_GLFW}")
message(STATUS "FreeType support: ${BGUI_USE_FREETYPE}")
message(STATUS "Build examples:   ${BGUI_BUILD_EXAMPLES}")


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