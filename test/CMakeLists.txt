enable_testing()
include(GoogleTest)

# Function to create test targets with common settings
function(add_test NAME)
    set(prefix "test_")
    set(target_name "${prefix}${NAME}")
    add_executable(${target_name} "${target_name}.cpp")
    target_link_libraries(${target_name} PRIVATE wbrcu GTest::gtest_main)
    gtest_discover_tests(${target_name})
endfunction()

add_test(rcu_protected)
add_test(rand)
