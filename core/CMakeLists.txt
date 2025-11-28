add_library(bgui_core STATIC)

file(GLOB_RECURSE BGUI_CORE_SOURCES source/*.cpp)

target_sources(bgui_core PRIVATE ${BGUI_CORE_SOURCES})

target_include_directories(bgui_core
    PUBLIC ${PROJECT_SOURCE_DIR}/core/include
)