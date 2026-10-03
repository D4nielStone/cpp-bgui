add_library(bgui_core STATIC)

file(GLOB_RECURSE BGUI_CORE_SOURCES CONFIGURE_DEPENDS source/*.cpp)

target_sources(bgui_core PRIVATE ${BGUI_CORE_SOURCES})

target_include_directories(bgui_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/core/include
)

target_compile_definitions(bgui_core
    PRIVATE BGUI_ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets"
)

if(BGUI_USE_FREETYPE)
    find_package(Freetype REQUIRED)
    target_compile_definitions(bgui_core PUBLIC BGUI_USE_FREETYPE)
    target_include_directories(bgui_core PUBLIC ${PROJECT_SOURCE_DIR}/backend)
    target_link_libraries(bgui_core PUBLIC Freetype::Freetype)
endif()

if(WIN32)
    # WIC provides native image decoding on Windows.
    target_link_libraries(bgui_core PRIVATE windowscodecs ole32)
else()
    # libpng provides PNG decoding on Linux and other non-Windows platforms.
    find_package(PNG REQUIRED)
    target_link_libraries(bgui_core PRIVATE PNG::PNG)
endif()