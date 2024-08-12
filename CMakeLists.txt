cmake_minimum_required(VERSION 3.16)

project(ASS)


# Library target
file(GLOB_RECURSE target_sources ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
set(target_name ass)
add_library(${target_name} INTERFACE ${target_sources})
target_include_directories(${target_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)


option(ASS_ENABLE_TESTING "Build with tests" OFF)
if (ASS_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
