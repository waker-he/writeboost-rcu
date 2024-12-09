# WriteBoost RCU

WriteBoost RCU is an enhanced Read-Copy-Update (RCU) library that provides integrated update-side synchronization while maintaining RCU's exceptional read performance. The library introduces several key innovations including a single updater/reclaimer design, batch processing of updates, and an optimized object pool for memory management. Through these improvements, WriteBoost RCU not only simplifies the programming model but also achieves superior performance across various scenarios.

## Library Usage (Interface)

The library provides a simple and intuitive interface through the `rcu_protected<T>` class template:

```cpp
rcu_protected<int> rp{new int(0)};

// Reader interface
void reader() {
    auto ptr = rp.get_ptr();
    doSomething(*ptr);
}

// Writer interface
void writer() {
    rp.update([](int* ptr) { doSomeUpdate(ptr); });
}
```

For detailed implementation and comprehensive benchmarking results, please refer to:
- [`final_report.pdf`](./final_report.pdf) - Full technical report with design details and evaluation
- [`include/wbrcu/rcu_protected.hpp`](./include/wbrcu/rcu_protected.hpp) - Implementation details

## Requirements

- C++20
- CMake 3.22 or higher

## Building and Running (Ubuntu 22.04)

To build and run on Ubuntu 22.04 (the platform used in our experiments):

```bash
# Install dependencies
sh install-deps.sh
conan install . --build=missing

# Build
cd build/Release
cmake ../.. -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Run tests
test/test_rcu_protected

# Run benchmarks
benchmark/bm_num_threads --benchmark_counters_tabular=true
benchmark/bm_workload --benchmark_counters_tabular=true
benchmark/bm_rw_ratio --benchmark_counters_tabular=true
benchmark/bm_sizeof_data --benchmark_counters_tabular=true
```

## Note for Grading

This project was completed as a team effort by 4 members. While the Git commit history primarily shows commits from Jiawei He, this does not reflect the actual distribution of work. Our team did not use Git branches for collaboration, instead we utilized VSCode Live Share (https://marketplace.visualstudio.com/items?itemName=MS-vsliveshare.vsliveshare) for real-time collaborative development. This allowed us to work together simultaneously on the codebase, with commits being consolidated and pushed by Jiawei He at major checkpoints. Each team member made substantial contributions to the project's design, implementation, and evaluation.