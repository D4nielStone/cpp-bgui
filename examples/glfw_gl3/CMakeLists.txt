cmake_minimum_required(VERSION 3.16)
project(example_glfw_gl3 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable(example_glfw_gl3
    main.cpp
)

target_include_directories(example_glfw_gl3
    PRIVATE ${PROJECT_SOURCE_DIR}/../../backend
            ${PROJECT_SOURCE_DIR}/../../core/include
)
target_link_libraries(example_glfw_gl3 PRIVATE bgui bgui_core bgui_backend)