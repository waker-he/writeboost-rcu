# WriteBoost RCU

## Build and Run

```sh
# install dependencies
sh install-deps.sh
conan install . --build=missing

# build
cd build/Release
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .

# run test
test/test_rcu_protected
test/test_rand

# run benchmark
benchmark/bm_num_threads --benchmark_counters_tabular=true
benchmark/bm_workload --benchmark_counters_tabular=true
benchmark/bm_rw_ratio --benchmark_counters_tabular=true
benchmark/bm_sizeof_data --benchmark_counters_tabular=true
```
