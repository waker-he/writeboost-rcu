
function(add_benchmark NAME)
    set(prefix "benchmark_")
    set(target_name "${prefix}${NAME}")
    add_executable(${target_name} "${target_name}.cpp")
    target_link_libraries(${target_name} PRIVATE wbrcu benchmark::benchmark_main)
endfunction()

add_benchmark(rcu_protected)