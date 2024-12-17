cmake_minimum_required(VERSION 3.16)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/generic_compile_options.cmake")

include(FetchContent)

file(GLOB_RECURSE target_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE target_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

option(ASS_FETCH_GOOGLE_TESTS "Disable it if you already have google tests in your project" ON)
option(ASS_FIND_GTEST_PACKAGE "Enable to make the script look for google tests in the system" OFF)

if (ASS_FETCH_GOOGLE_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG        "release-1.12.1"
    )
    
  FetchContent_MakeAvailable(googletest)
elseif(ASS_FIND_GTEST_PACKAGE)
  find_package(GTest REQUIRED)
  add_library(gtest ALIAS GTest::gtest)
  add_library(gtest_main ALIAS GTest::gtest_main)
endif()

set(target_name ass_tests)
add_executable(${target_name} ${target_headers} ${target_sources})
target_link_libraries(${target_name} PUBLIC gtest_main ass)
set_generic_compile_options(${target_name} INTERFACE)

enable_testing()
include(GoogleTest)
gtest_discover_tests(${target_name})

if(MSVC)
  target_compile_options(${target_name} PRIVATE /bigobj)
endif()
