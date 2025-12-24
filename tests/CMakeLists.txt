include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG      main 
)

FetchContent_MakeAvailable(googletest)

add_executable(
    elem_test
    test_elem.cpp
)

target_include_directories(elem_test
    PRIVATE ${PROJECT_SOURCE_DIR}/core/include
)

target_link_libraries(
    elem_test PRIVATE 
    bgui
    GTest::gtest_main 
)

add_test(NAME ElementTest COMMAND elem_test)