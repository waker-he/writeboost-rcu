cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
add_compile_options(-Wall -Wextra)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(WRITE_BOOST_RCU LANGUAGES CXX)

include(cmake/clang_format.cmake)

# get hardware_concurrency as a compile-time constant
include(ProcessorCount)
ProcessorCount(N)

if(N EQUAL 0)
    message(FATAL_ERROR "Could not determine processor count")
endif()

add_compile_definitions(WBRCU_HARDWARE_CONCURRENCY=${N})

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -flto -ffast-math -march=native")

# Dependencies
find_package(folly REQUIRED)
find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)

# -------------------------------------------------------------------
#                       Library definition
add_library(wbrcu INTERFACE)
set(WBRCU_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
target_include_directories(wbrcu INTERFACE ${WBRCU_INCLUDE_DIR})
target_compile_features(wbrcu INTERFACE cxx_std_20)
target_link_libraries(wbrcu INTERFACE folly::folly)


add_subdirectory(test)
add_subdirectory(benchmark)