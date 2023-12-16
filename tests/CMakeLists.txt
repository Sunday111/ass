cmake_minimum_required(VERSION 3.16)

include(FetchContent)

file(GLOB_RECURSE target_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE target_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG        "release-1.12.1"
)

FetchContent_MakeAvailable(googletest)

set(target_name ass_tests)
add_executable(${target_name} ${target_headers} ${target_sources})
target_link_libraries(${target_name} PUBLIC gtest_main ass)

enable_testing()
include(GoogleTest)
gtest_discover_tests(${target_name})
