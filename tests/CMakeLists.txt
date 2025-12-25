include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG      main 
)

FetchContent_MakeAvailable(googletest)

add_executable(
    all_tests
    all_tests.cpp
)

target_include_directories(all_tests
    PRIVATE ${PROJECT_SOURCE_DIR}/core/include
)

target_link_libraries(
    all_tests PRIVATE 
    bgui
    GTest::gtest_main 
)

add_test(NAME ElementTest COMMAND all_tests)