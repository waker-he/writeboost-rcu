# benchmark against folly's implementation
find_package(folly REQUIRED)

add_executable(test test.cpp)
target_link_libraries(test PRIVATE wbrcu folly::folly)