cmake_minimum_required(VERSION 3.20)
include(set_compiler_options)
set(module_source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/ass/bit/bit_count_to_type.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/ass/bit_span.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/ass/enum/enum_as_index.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/ass/enum/enum_as_index_magic_enum.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/ass/enum_map.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/ass/enum_set.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/ass/fixed_bitset.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/ass/fixed_unordered_map.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/ass/invalid_index.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/code/public/ass/macro/empty_bases.hpp)
add_library(ass INTERFACE ${module_source_files})
set_generic_compiler_options(ass INTERFACE)
target_include_directories(ass INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/code/public)
target_include_directories(ass INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/code/private)
