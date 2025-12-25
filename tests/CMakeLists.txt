include(FetchContent)
cmake_policy(SET CMP0135 NEW)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(unit_tests
        directories.cpp
)

target_link_libraries(unit_tests PRIVATE
        fs_core
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
