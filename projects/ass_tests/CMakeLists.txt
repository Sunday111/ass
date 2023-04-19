cmake_minimum_required(VERSION 3.16)

include(FetchContent)

file(GLOB_RECURSE target_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG        main
)

FetchContent_MakeAvailable(googletest)

set(target_name ass_tests)
add_executable(${target_name} ${target_sources})
target_link_libraries(${target_name} PUBLIC gtest_main ass)

# get_cmake_property(_variableNames VARIABLES)
# list(SORT _variableNames)

# foreach(_variableName ${_variableNames})
#     message("${_variableName}=${${_variableName}}")
# endforeach()