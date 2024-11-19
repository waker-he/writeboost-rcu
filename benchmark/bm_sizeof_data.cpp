#include <thread>
#include <array>
#include <iostream>

#include "common.hpp"
#include "wbrcu/rcu_protected.hpp"
#include "benchmark/benchmark.h"

// Data structure with varying size
template <size_t N>
struct ArrayData {
    std::array<uint64_t, N> data{};
};

template <template<typename> class Protect, size_t N>
class BMSizeOfDataFixture : public benchmark::Fixture {
public:
    size_t sz = N;
    Protect<ArrayData<N>> p{new ArrayData<N>{}};
};

constexpr static int write_iterations = 100;

void bm_func(benchmark::State& state, size_t sz, auto& p) {
    uint64_t write_ops = 0;
    auto const mask = sz - 1;
    for (auto _ : state) {
        for (int i = 0; i < write_iterations; ++i) {
            size_t index = rand() & mask;
            p.update([index](auto* ptr) { ++(ptr->data[index]); });
        }
        write_ops += write_iterations;
    }
    state.counters["total_write_ops"] = benchmark::Counter(write_ops * state.threads(), benchmark::Counter::kIsRate);;
}

// 2
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, WBRCU_Size2, wbrcu::rcu_protected, 2)(benchmark::State& state) {
    bm_func(state, sz, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, FollyRCU_Size2, follyrcu_protected, 2)(benchmark::State& state) {
    bm_func(state, sz, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, SharedMutex_Size2, rwlock_protected, 2)(benchmark::State& state) {
    bm_func(state, sz, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, Mutex_Size2, lock_protected, 2)(benchmark::State& state) {
    bm_func(state, sz, p); 
}

BENCHMARK_REGISTER_F(BMSizeOfDataFixture, WBRCU_Size2)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, FollyRCU_Size2)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, SharedMutex_Size2)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, Mutex_Size2)->Threads(WBRCU_HARDWARE_CONCURRENCY);

// 8
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, WBRCU_Size8, wbrcu::rcu_protected, 8)(benchmark::State& state) {
    bm_func(state, sz, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, FollyRCU_Size8, follyrcu_protected, 8)(benchmark::State& state) {
    bm_func(state, sz, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, SharedMutex_Size8, rwlock_protected, 8)(benchmark::State& state) {
    bm_func(state, sz, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, Mutex_Size8, lock_protected, 8)(benchmark::State& state) {
    bm_func(state, sz, p);
}

BENCHMARK_REGISTER_F(BMSizeOfDataFixture, WBRCU_Size8)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, FollyRCU_Size8)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, SharedMutex_Size8)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, Mutex_Size8)->Threads(WBRCU_HARDWARE_CONCURRENCY);

// 16
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, WBRCU_Size16, wbrcu::rcu_protected, 16)(benchmark::State& state) {
    bm_func(state, sz, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, FollyRCU_Size16, follyrcu_protected, 16)(benchmark::State& state) {
    bm_func(state, sz, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, SharedMutex_Size16, rwlock_protected, 16)(benchmark::State& state) {
    bm_func(state, sz, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, Mutex_Size16, lock_protected, 16)(benchmark::State& state) {
    bm_func(state, sz, p);
}

BENCHMARK_REGISTER_F(BMSizeOfDataFixture, WBRCU_Size16)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, FollyRCU_Size16)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, SharedMutex_Size16)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, Mutex_Size16)->Threads(WBRCU_HARDWARE_CONCURRENCY);


// 64
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, WBRCU_Size64, wbrcu::rcu_protected, 64)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, FollyRCU_Size64, follyrcu_protected, 64)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, SharedMutex_Size64, rwlock_protected, 64)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, Mutex_Size64, lock_protected, 64)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, WBRCU_Size64)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, FollyRCU_Size64)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, SharedMutex_Size64)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, Mutex_Size64)->Threads(WBRCU_HARDWARE_CONCURRENCY);

// 256
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, WBRCU_Size256, wbrcu::rcu_protected, 256)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, FollyRCU_Size256, follyrcu_protected, 256)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, SharedMutex_Size256, rwlock_protected, 256)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, Mutex_Size256, lock_protected, 256)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, WBRCU_Size256)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, FollyRCU_Size256)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, SharedMutex_Size256)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, Mutex_Size256)->Threads(WBRCU_HARDWARE_CONCURRENCY);

// 1024
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, WBRCU_Size1024, wbrcu::rcu_protected, 1024)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, FollyRCU_Size1024, follyrcu_protected, 1024)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, SharedMutex_Size1024, rwlock_protected, 1024)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, Mutex_Size1024, lock_protected, 1024)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, WBRCU_Size1024)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, FollyRCU_Size1024)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, SharedMutex_Size1024)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, Mutex_Size1024)->Threads(WBRCU_HARDWARE_CONCURRENCY);

// 4096
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, WBRCU_Size4096, wbrcu::rcu_protected, 4096)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, FollyRCU_Size4096, follyrcu_protected, 4096)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, SharedMutex_Size4096, rwlock_protected, 4096)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, Mutex_Size4096, lock_protected, 4096)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, WBRCU_Size4096)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, FollyRCU_Size4096)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, SharedMutex_Size4096)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, Mutex_Size4096)->Threads(WBRCU_HARDWARE_CONCURRENCY);

// 16384
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, WBRCU_Size16384, wbrcu::rcu_protected, 16384)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, FollyRCU_Size16384, follyrcu_protected, 16384)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, SharedMutex_Size16384, rwlock_protected, 16384)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, Mutex_Size16384, lock_protected, 16384)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, WBRCU_Size16384)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, FollyRCU_Size16384)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, SharedMutex_Size16384)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, Mutex_Size16384)->Threads(WBRCU_HARDWARE_CONCURRENCY);

// 65536
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, WBRCU_Size65536, wbrcu::rcu_protected, 65536)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, FollyRCU_Size65536, follyrcu_protected, 65536)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, SharedMutex_Size65536, rwlock_protected, 65536)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_TEMPLATE_DEFINE_F(BMSizeOfDataFixture, Mutex_Size65536, lock_protected, 65536)(benchmark::State& state) {
    bm_func(state, sz, p);
}
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, WBRCU_Size65536)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, FollyRCU_Size65536)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, SharedMutex_Size65536)->Threads(WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(BMSizeOfDataFixture, Mutex_Size65536)->Threads(WBRCU_HARDWARE_CONCURRENCY);