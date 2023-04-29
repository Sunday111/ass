cmake_minimum_required(VERSION 3.16)

project(ASS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(code/ass)
add_subdirectory(code/ass_tests)