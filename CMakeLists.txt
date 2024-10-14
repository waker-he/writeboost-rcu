cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
add_compile_options(-Wall -Wextra)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(WRITE_BOOST_RCU LANGUAGES CXX)

include(cmake/clang_format.cmake)

# Dependencies
find_package(folly REQUIRED)
find_package(Boost REQUIRED)

# -------------------------------------------------------------------
#                       Library definition
add_library(wbrcu INTERFACE)
set(WBRCU_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
target_include_directories(wbrcu INTERFACE ${WBRCU_INCLUDE_DIR})
target_compile_features(wbrcu INTERFACE cxx_std_20)
target_link_libraries(wbrcu INTERFACE folly::folly boost::boost)


add_subdirectory(test)