#include <thread>
#include <vector>
#include <iostream>

#include "common.hpp"
#include "wbrcu/rcu_protected.hpp"
#include "benchmark/benchmark.h"

template <class ProtectedType>
class WBRCUFixture : public benchmark::Fixture {
public:
    wbrcu::rcu_protected<ProtectedType> p{new ProtectedType{}};
};

template <class ProtectedType>
class FollyRCUFixture : public benchmark::Fixture {
public:
    follyrcu_protected<ProtectedType> p{new ProtectedType{}};
};

template <class ProtectedType>
class SharedMutexFixture : public benchmark::Fixture {
public:
    rwlock_protected<ProtectedType> p{new ProtectedType{}};
};

template <class ProtectedType>
class MutexFixture : public benchmark::Fixture {
public:
    lock_protected<ProtectedType> p{new ProtectedType{}};
};

void simulate_work(int nanoseconds) {
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::high_resolution_clock::now() - start < std::chrono::nanoseconds(nanoseconds)) {
    }
}

constexpr static int read_iterations = 1000;

BENCHMARK_TEMPLATE_DEFINE_F(WBRCUFixture, WBRCU_ProtectInt_Reader, uint64_t)(benchmark::State& state) {
    uint64_t read_ops = 0;
    for (auto _ : state) {
        int k;
        for (int i = 0; i < read_iterations; ++i) {
            auto ptr = p.get_ptr();
            benchmark::DoNotOptimize(k = *ptr);
            simulate_work(state.range(0));
        }
        read_ops += read_iterations;
    }
    state.counters["read_ops_per_thread"] = benchmark::Counter(read_ops, benchmark::Counter::kIsRate);;
}

BENCHMARK_TEMPLATE_DEFINE_F(FollyRCUFixture, FollyRCU_ProtectInt_Reader, uint64_t)(benchmark::State& state) {
    uint64_t read_ops = 0;
    for (auto _ : state) {
        int k;
        for (int i = 0; i < read_iterations; ++i) {
            auto ptr = p.get_ptr();
            benchmark::DoNotOptimize(k = *ptr);
            simulate_work(state.range(0));
        }
        read_ops += read_iterations;
    }
    state.counters["read_ops_per_thread"] = benchmark::Counter(read_ops, benchmark::Counter::kIsRate);;
}

BENCHMARK_TEMPLATE_DEFINE_F(SharedMutexFixture, SharedMutex_ProtectInt_Reader, uint64_t)(benchmark::State& state) {
    uint64_t read_ops = 0;
    for (auto _ : state) {
        int k;
        for (int i = 0; i < read_iterations; ++i) {
            auto ptr = p.get_ptr();
            benchmark::DoNotOptimize(k = *ptr);
            simulate_work(state.range(0));
        }
        read_ops += read_iterations;
    }
    state.counters["read_ops_per_thread"] = benchmark::Counter(read_ops, benchmark::Counter::kIsRate);;
}

BENCHMARK_TEMPLATE_DEFINE_F(MutexFixture, Mutex_ProtectInt_Reader, uint64_t)(benchmark::State& state) {
    uint64_t read_ops = 0;
    for (auto _ : state) {
        int k;
        for (int i = 0; i < read_iterations; ++i) {
            auto ptr = p.get_ptr();
            benchmark::DoNotOptimize(k = *ptr);
            simulate_work(state.range(0));
        }
        read_ops += read_iterations;
    }
    state.counters["read_ops_per_thread"] = benchmark::Counter(read_ops, benchmark::Counter::kIsRate);;
}

constexpr int lower_ns = 10;
constexpr int upper_ns = 10000;

BENCHMARK_REGISTER_F(WBRCUFixture, WBRCU_ProtectInt_Reader)->Threads(WBRCU_HARDWARE_CONCURRENCY)->RangeMultiplier(10)->Range(lower_ns, upper_ns);
BENCHMARK_REGISTER_F(FollyRCUFixture, FollyRCU_ProtectInt_Reader)->Threads(WBRCU_HARDWARE_CONCURRENCY)->RangeMultiplier(10)->Range(lower_ns, upper_ns);
BENCHMARK_REGISTER_F(SharedMutexFixture, SharedMutex_ProtectInt_Reader)->Threads(WBRCU_HARDWARE_CONCURRENCY)->RangeMultiplier(10)->Range(lower_ns, upper_ns);
BENCHMARK_REGISTER_F(MutexFixture, Mutex_ProtectInt_Reader)->Threads(WBRCU_HARDWARE_CONCURRENCY)->RangeMultiplier(10)->Range(lower_ns, upper_ns);
