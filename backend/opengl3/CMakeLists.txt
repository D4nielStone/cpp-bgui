add_library(bgui_backend_opengl3 STATIC bgui_backend_opengl3.cpp)

target_compile_definitions(bgui_backend_opengl3 PUBLIC BGUI_USE_OPENGL)

find_package(GLEW REQUIRED)
target_link_libraries(bgui_backend_opengl3 PUBLIC GLEW::GLEW)
target_include_directories(bgui_backend_opengl3
    PUBLIC ${PROJECT_SOURCE_DIR}/backend
    ${PROJECT_SOURCE_DIR}/core/include
)