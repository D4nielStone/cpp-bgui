add_library(bgui_core STATIC)

file(GLOB_RECURSE BGUI_CORE_SOURCES source/*.cpp)

target_sources(bgui_core PRIVATE ${BGUI_CORE_SOURCES})

target_include_directories(bgui_core
    PUBLIC ${PROJECT_SOURCE_DIR}/core/include
)

target_compile_definitions(bgui_core
    PRIVATE BGUI_ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets"
)

if(WIN32)
    # WIC provides native image decoding on Windows.
    target_link_libraries(bgui_core PRIVATE windowscodecs ole32)
else()
    # libpng provides PNG decoding on Linux and other non-Windows platforms.
    find_package(PNG REQUIRED)
    target_link_libraries(bgui_core PRIVATE PNG::PNG)
endif()