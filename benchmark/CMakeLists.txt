
function(add_benchmark NAME)
    set(prefix "bm_")
    set(target_name "${prefix}${NAME}")
    add_executable(${target_name} "${target_name}.cpp")
    target_link_libraries(${target_name} PRIVATE wbrcu benchmark::benchmark_main)
endfunction()

add_benchmark(num_threads)
add_benchmark(workload)
add_benchmark(sizeof_data)
add_benchmark(rw_ratio)
add_benchmark(rw_ratio2)