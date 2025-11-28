cmake_minimum_required(VERSION 3.16)
project(exemple_glfw_opengl3 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable(exemple_glfw_opengl3
    main.cpp
)

target_include_directories(exemple_glfw_opengl3
    PRIVATE ${PROJECT_SOURCE_DIR}/../../backend
            ${PROJECT_SOURCE_DIR}/../../core/include
)
target_link_libraries(exemple_glfw_opengl3 PRIVATE bgui)