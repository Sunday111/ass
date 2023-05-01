cmake_minimum_required(VERSION 3.16)

project(ASS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Library target

file(GLOB_RECURSE target_sources ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

set(target_name ass)
add_library(${target_name} INTERFACE ${target_sources})
target_include_directories(${target_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(generic_compile_options)
set_generic_compile_options(${target_name} INTERFACE)

add_subdirectory(tests)
