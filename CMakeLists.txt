cmake_minimum_required(VERSION 3.29)

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


set(YAE_STAGING_ROOT "${CMAKE_BINARY_DIR}/.yae-staging")
set(YAE_STAGING_PLAN "${YAE_STAGING_ROOT}/active-manifests.txt")
file(GLOB_RECURSE YAE_LEGACY_STAGING_MANIFESTS
    LIST_DIRECTORIES FALSE
    "${CMAKE_BINARY_DIR}/yae_modules/*_copy_files_*.manifest")
if(EXISTS "${YAE_STAGING_ROOT}" OR YAE_LEGACY_STAGING_MANIFESTS)
    find_package(Python3 3.12 REQUIRED COMPONENTS Interpreter)
    execute_process(
        COMMAND ${Python3_EXECUTABLE}
                "${YAE_SUPPORT_ROOT}/scripts/stage_directories.py"
                --write-plan
                --manifest-root "${YAE_STAGING_ROOT}"
                --active-manifests "${YAE_STAGING_PLAN}"
        RESULT_VARIABLE YAE_STAGING_PLAN_RESULT
        ERROR_VARIABLE YAE_STAGING_PLAN_ERROR)
    if(NOT YAE_STAGING_PLAN_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to publish staging plan: ${YAE_STAGING_PLAN_ERROR}")
    endif()
    add_custom_target(yae_reconcile_staging ALL
        COMMAND ${Python3_EXECUTABLE} "${YAE_SUPPORT_ROOT}/scripts/stage_directories.py"
                --reconcile
                --destination-root "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                --manifest-root "${YAE_STAGING_ROOT}"
                --active-manifests "${YAE_STAGING_PLAN}"
                --legacy-manifest-root "${CMAKE_BINARY_DIR}/yae_modules"
        COMMENT "reconcile staged content"
        VERBATIM)
endif()


set(YAE_ass_SOURCES modules/ass)
add_subdirectory(${YAE_ass_SOURCES} yae_modules/modules/ass SYSTEM)

# https://github.com/google/googletest v1.18.0
set(YAE_gtest_main_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/google/googletest/v1.18.0)
option(INSTALL_GTEST "" OFF)
option(BUILD_GMOCK "" OFF)
add_subdirectory(${YAE_gtest_main_SOURCES} yae_modules/google/googletest/v1.18.0 SYSTEM)

set(YAE_AssTests_SOURCES modules/AssTests)
add_subdirectory(${YAE_AssTests_SOURCES} yae_modules/modules/AssTests SYSTEM)

# https://github.com/fmtlib/fmt 12.2.0
set(YAE_fmt_header_only_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/fmtlib/fmt/12.2.0)
add_subdirectory(${YAE_fmt_header_only_SOURCES} yae_modules/fmtlib/fmt/12.2.0 SYSTEM)

# https://github.com/google/benchmark v1.9.5
set(YAE_gbench_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/google/benchmark/v1.9.5)
option(BENCHMARK_ENABLE_TESTING "" OFF)
add_subdirectory(${YAE_gbench_SOURCES} yae_modules/google/benchmark/v1.9.5 SYSTEM)

# https://github.com/martinus/unordered_dense v4.9.2
set(YAE_unordered_dense_SOURCES ${YAE_CLONED_REPOSITORIES_DIR}/martinus/unordered_dense/v4.9.2)
add_subdirectory(${YAE_unordered_dense_SOURCES} yae_modules/martinus/unordered_dense/v4.9.2 SYSTEM)

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

if(TARGET yae_reconcile_staging AND TARGET ass)
    add_dependencies(ass yae_reconcile_staging)
endif()
if(TARGET yae_reconcile_staging AND TARGET AssTests)
    add_dependencies(AssTests yae_reconcile_staging)
endif()
if(TARGET yae_reconcile_staging AND TARGET yae_example_benchmark)
    add_dependencies(yae_example_benchmark yae_reconcile_staging)
endif()
if(TARGET yae_reconcile_staging AND TARGET yae_example_benchmark_main)
    add_dependencies(yae_example_benchmark_main yae_reconcile_staging)
endif()
if(TARGET yae_reconcile_staging AND TARGET yae_example_library)
    add_dependencies(yae_example_library yae_reconcile_staging)
endif()
if(TARGET yae_reconcile_staging AND TARGET yae_example_header_only)
    add_dependencies(yae_example_header_only yae_reconcile_staging)
endif()
if(TARGET yae_reconcile_staging AND TARGET yae_example_executable)
    add_dependencies(yae_example_executable yae_reconcile_staging)
endif()
if(TARGET yae_reconcile_staging AND TARGET yae_example_tests)
    add_dependencies(yae_example_tests yae_reconcile_staging)
endif()
if(TARGET yae_reconcile_staging AND TARGET yae_example_tests_main)
    add_dependencies(yae_example_tests_main yae_reconcile_staging)
endif()

enable_testing()
