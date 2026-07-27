cmake_minimum_required(VERSION 3.20)

project(ass)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Set output directories for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
# Set output directories for archives
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
# Set output directories for libraries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Set path to cloned repositories sources
set(YAE_CLONED_REPOSITORIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cloned_repositories" CACHE PATH "Path to YAE cloned repository checkouts")

set(YAE_SUPPORT_ROOT "${YAE_CLONED_REPOSITORIES_DIR}/Sunday111/yae-support/main")
set(YAE_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${YAE_SUPPORT_ROOT}/cmake")




set(YAE_ass_SOURCES modules/ass)
add_subdirectory(${YAE_ass_SOURCES} yae_modules/modules/ass SYSTEM)

# https://github.com/google/googletest v1.15.2
set(YAE_gtest_main_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/google/googletest/v1.15.2)
option(INSTALL_GTEST "" OFF)
option(BUILD_GMOCK "" OFF)
add_subdirectory(${YAE_gtest_main_SOURCES} yae_modules/google/googletest/v1.15.2 SYSTEM)

set(YAE_AssTests_SOURCES modules/AssTests)
add_subdirectory(${YAE_AssTests_SOURCES} yae_modules/modules/AssTests SYSTEM)

# https://github.com/fmtlib/fmt 11.1.1
set(YAE_fmt_header_only_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/fmtlib/fmt/11.1.1)
add_subdirectory(${YAE_fmt_header_only_SOURCES} yae_modules/fmtlib/fmt/11.1.1 SYSTEM)

# https://github.com/google/benchmark v1.9.4
set(YAE_gbench_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/google/benchmark/v1.9.4)
option(BENCHMARK_ENABLE_TESTING "" OFF)
add_subdirectory(${YAE_gbench_SOURCES} yae_modules/google/benchmark/v1.9.4 SYSTEM)

include(${YAE_CLONED_REPOSITORIES_DIR}/Sunday111/yae-support/main/modules/third_party/gbench.module.cmake)
# https://github.com/martinus/unordered_dense v4.1.2
set(YAE_unordered_dense_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/martinus/unordered_dense/v4.1.2)
add_subdirectory(${YAE_unordered_dense_SOURCES} yae_modules/martinus/unordered_dense/v4.1.2 SYSTEM)

set(YAE_yae_example_benchmark_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/Sunday111/yae-support/main/modules/examples/yae_example_benchmark)
add_subdirectory(${YAE_yae_example_benchmark_SOURCES} yae_modules/Sunday111/yae-support/main/modules/examples/yae_example_benchmark SYSTEM)

set(YAE_yae_example_benchmark_main_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/Sunday111/yae-support/main/modules/examples/yae_example_benchmark_main)
add_subdirectory(${YAE_yae_example_benchmark_main_SOURCES} yae_modules/Sunday111/yae-support/main/modules/examples/yae_example_benchmark_main SYSTEM)

set(YAE_yae_example_library_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/Sunday111/yae-support/main/modules/examples/yae_example_library)
add_subdirectory(${YAE_yae_example_library_SOURCES} yae_modules/Sunday111/yae-support/main/modules/examples/yae_example_library SYSTEM)

set(YAE_yae_example_header_only_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/Sunday111/yae-support/main/modules/examples/yae_example_header_only)
add_subdirectory(${YAE_yae_example_header_only_SOURCES} yae_modules/Sunday111/yae-support/main/modules/examples/yae_example_header_only SYSTEM)

set(YAE_yae_example_executable_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/Sunday111/yae-support/main/modules/examples/yae_example_executable)
add_subdirectory(${YAE_yae_example_executable_SOURCES} yae_modules/Sunday111/yae-support/main/modules/examples/yae_example_executable SYSTEM)

set(YAE_yae_example_tests_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/Sunday111/yae-support/main/modules/examples/yae_example_tests)
add_subdirectory(${YAE_yae_example_tests_SOURCES} yae_modules/Sunday111/yae-support/main/modules/examples/yae_example_tests SYSTEM)

set(YAE_yae_example_tests_main_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/Sunday111/yae-support/main/modules/examples/yae_example_tests_main)
add_subdirectory(${YAE_yae_example_tests_main_SOURCES} yae_modules/Sunday111/yae-support/main/modules/examples/yae_example_tests_main SYSTEM)


enable_testing()
