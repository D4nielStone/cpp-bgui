include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG      v1.15.2
)

FetchContent_MakeAvailable(googletest)

add_executable(
    all_tests
    audit_tests.cpp
    element_tests.cpp
    menu_bar_tests.cpp
    inputbox_tests.cpp
    combo_box_tests.cpp
    tree_tests.cpp
    value_control_tests.cpp
    vector_field_tests.cpp
    image_tests.cpp
    linear_tests.cpp
    dock_tests.cpp
    style_tests.cpp
    profiling_tests.cpp
    icon_tests.cpp
)

target_include_directories(all_tests
    PRIVATE ${PROJECT_SOURCE_DIR}/core/include
)

target_compile_definitions(all_tests PRIVATE
  $<$<CONFIG:Debug>:BGUI_LOGS_DEFAULT_ENABLED>
)

target_link_libraries(
    all_tests PRIVATE 
    bgui
    GTest::gtest_main 
)

add_test(NAME AuditTests COMMAND all_tests --gtest_filter=AuditTest.*)
add_test(NAME ElementTests COMMAND all_tests --gtest_filter=ElementTest.*)
add_test(NAME MenuBarTests COMMAND all_tests --gtest_filter=MenuBarTest.*)
add_test(NAME InputAreaTests COMMAND all_tests --gtest_filter=InputAreaTest.*)
add_test(NAME ValueControlTests COMMAND all_tests --gtest_filter=ValueControlTest.*)
add_test(NAME VectorFieldTests COMMAND all_tests --gtest_filter=VectorFieldTest.*)
add_test(NAME ContextMenuTests COMMAND all_tests --gtest_filter=ContextMenuTest.*)
add_test(NAME ComboBoxTests COMMAND all_tests --gtest_filter=ComboBoxTest.*)
add_test(NAME treeTests COMMAND all_tests --gtest_filter=treeTest.*)
add_test(NAME InteractionTests COMMAND all_tests --gtest_filter=InteractionTest.*)
add_test(NAME ConfigurationTests COMMAND all_tests --gtest_filter=ConfigurationTest.*)
add_test(NAME ImageTests COMMAND all_tests --gtest_filter=ImageTest.*)
add_test(NAME LinearTests COMMAND all_tests --gtest_filter=LinearTest.*)
add_test(NAME DockTests COMMAND all_tests --gtest_filter=DockTest.*)
add_test(NAME StyleTests COMMAND all_tests --gtest_filter=StyleVisualTest.*:DeclarativeStyleTest.*:ModalThemeTest.*)
add_test(NAME DrawTests COMMAND all_tests --gtest_filter=DrawDataTest.*:DrawListTest.*)
add_test(NAME ProfilingTests COMMAND all_tests --gtest_filter=ProfilingTest.*)
add_test(NAME IconTests COMMAND all_tests --gtest_filter=IconTest.*)

if(BGUI_USE_OPENGL AND BGUI_USE_GLFW)
  add_executable(gl3_tests gl3_tests.cpp)
  target_link_libraries(gl3_tests PRIVATE bgui GTest::gtest_main)
  add_test(NAME GL3Tests COMMAND gl3_tests)
endif()

if(WIN32)
  # On Windows, do not link pthreads
  set(THREADS_PREFER_PTHREAD_FLAG OFF)
  find_package(Threads REQUIRED)
  target_link_libraries(all_tests PRIVATE Threads::Threads)
else()
  # On Unix-like systems, you may need pthreads
  find_package(Threads REQUIRED)
  target_link_libraries(all_tests PRIVATE Threads::Threads)
endif()
