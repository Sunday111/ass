cmake_minimum_required(VERSION 3.20)
include(set_compiler_options)
set(module_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/bit/bit_count_to_type_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/bit_span_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/enum/enum_as_index.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/enum_map_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/enum_set_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/fixed_bitset_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/fixed_map_tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/private/test_helpers.hpp)
add_executable(AssTests ${module_source_files})
set_generic_compiler_options(AssTests PRIVATE)
target_link_libraries(AssTests PRIVATE ass
                                       gtest_main)
target_include_directories(AssTests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/code/public)
target_include_directories(AssTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/code/private)
enable_testing()
include(GoogleTest)
gtest_discover_tests(AssTests)
