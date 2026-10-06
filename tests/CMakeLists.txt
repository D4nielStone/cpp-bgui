include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG      v1.15.2
)

FetchContent_MakeAvailable(googletest)

add_executable(
    all_tests
    element_tests.cpp
    inputbox_tests.cpp
    combo_box_tests.cpp
    tree_tests.cpp
    value_control_tests.cpp
    image_tests.cpp
    linear_tests.cpp
    dock_tests.cpp
    style_tests.cpp
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

add_test(NAME ElementTests COMMAND all_tests --gtest_filter=ElementTest.*)
add_test(NAME InputAreaTests COMMAND all_tests --gtest_filter=InputAreaTest.*)
add_test(NAME ValueControlTests COMMAND all_tests --gtest_filter=ValueControlTest.*)
add_test(NAME ComboBoxTests COMMAND all_tests --gtest_filter=ComboBoxTest.*)
add_test(NAME treeTests COMMAND all_tests --gtest_filter=treeTest.*)
add_test(NAME InteractionTests COMMAND all_tests --gtest_filter=InteractionTest.*)
add_test(NAME ConfigurationTests COMMAND all_tests --gtest_filter=ConfigurationTest.*)
add_test(NAME ImageTests COMMAND all_tests --gtest_filter=ImageTest.*)
add_test(NAME LinearTests COMMAND all_tests --gtest_filter=LinearTest.*)
add_test(NAME DockTests COMMAND all_tests --gtest_filter=DockTest.*)
add_test(NAME StyleTests COMMAND all_tests --gtest_filter=StyleVisualTest.*:DeclarativeStyleTest.*)

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