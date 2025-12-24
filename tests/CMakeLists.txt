include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG      main 
)

FetchContent_MakeAvailable(googletest)


add_executable(
    unit_tests
    test_elem.cpp
)

target_link_libraries(
    unit_tests PRIVATE 
    bgui
    GTest::gtest_main 
    GTest::gtest
)

target_include_directories(unit_tests
    PUBLIC ${PROJECT_SOURCE_DIR}/core/include
)