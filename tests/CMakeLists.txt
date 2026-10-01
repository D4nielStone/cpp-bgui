include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG      main 
)

FetchContent_MakeAvailable(googletest)

add_executable(
    all_tests
    element_tests.cpp
    image_tests.cpp
    linear_tests.cpp
    style_tests.cpp
)

target_include_directories(all_tests
    PRIVATE ${PROJECT_SOURCE_DIR}/core/include
)

target_link_libraries(
    all_tests PRIVATE 
    bgui
    GTest::gtest_main 
)

add_test(NAME ElementTests COMMAND all_tests --gtest_filter=ElementTest.*)
add_test(NAME ImageTests COMMAND all_tests --gtest_filter=ImageTest.*)
add_test(NAME LinearTests COMMAND all_tests --gtest_filter=LinearTest.*)
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