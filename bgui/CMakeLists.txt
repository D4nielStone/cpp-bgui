cmake_minimum_required(VERSION 3.28)
# set the name of the project
project(bgui VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set the source of the project
add_library(bgui STATIC
    source/bgui.cpp
    source/elem/element.cpp
    source/elem/absolute_layout.cpp
    source/elem/text.cpp
    source/os/os.cpp
    source/os/font.cpp
    source/gl/shader.cpp
    source/utils/material.cpp
    ) 

include_directories(
    ${CMAKE_SOURCE_DIR}/libs/include
    ${CMAKE_SOURCE_DIR}/bgui/include
    )

target_include_directories(glad PUBLIC
    ${CMAKE_SOURCE_DIR}/libs/include
    ${CMAKE_SOURCE_DIR}/libs/include/glad
)

# platform-specific include directories
if(WIN32)
    message(STATUS "Configuring for Windows (WIN32)")
    set(GLFW_LIB "${CMAKE_SOURCE_DIR}/libs/lib/libglfw3.a")
    set(FT_LIB "${CMAKE_SOURCE_DIR}/libs/lib/libfreetype.a")

    if(NOT EXISTS ${GLFW_LIB})
        message(FATAL_ERROR "GLFW static/import library not found: ${GLFW_LIB}")
    endif()

    if(NOT EXISTS ${FT_LIB})
        message(FATAL_ERROR "FreeType static/import library not found: ${GLFW_LIB}")
    endif()

    target_include_directories(bgui PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/include
        ${CMAKE_SOURCE_DIR}/libs/include/GLFW
        ${CMAKE_SOURCE_DIR}/libs/include/glad
    )
    target_link_options(bgui PRIVATE "-mwindows")
    # Link by full path and add required system libs
    target_link_libraries(bgui PRIVATE
        ${GLFW_LIB}
        ${FT_LIB}
        glad
        opengl32
        gdi32
        user32
        kernel32
        ws2_32
        winmm
    )
else()
    message(STATUS "Configuring for Linux")
    # default includes for unix-like systems
    include_directories(
        ${CMAKE_SOURCE_DIR}/libs/include
        ${CMAKE_SOURCE_DIR}/bgui/include
    )
    
    target_include_directories(bgui PUBLIC
        ${CMAKE_SOURCE_DIR}/libs/include/glad
        ${CMAKE_SOURCE_DIR}/libs/include/
    )
    
    target_link_libraries(bgui PUBLIC
        glad
        glfw
        freetype
    )
endif()
