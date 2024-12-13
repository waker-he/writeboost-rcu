#include <thread>
#include <vector>
#include <iostream>
#include <pthread.h>
#include <sched.h>
#include <numa.h>
#include <numaif.h>
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
void bind_thread_to_node(int numa_node) {
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);

    struct bitmask *cpus = numa_allocate_cpumask();
    numa_node_to_cpus(numa_node, cpus);

    for (int i = 0; i < cpus->size; ++i) {
        if (numa_bitmask_isbitset(cpus, i)) {
            CPU_SET(i, &cpu_set);
        }
    }

    pthread_t thread = pthread_self();
    if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpu_set) != 0) {
        std::cerr << "Failed to set thread affinity to NUMA node " << numa_node << "\n";
    }

    numa_free_cpumask(cpus);
}
void simulate_work(int nanoseconds) {
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::high_resolution_clock::now() - start < std::chrono::nanoseconds(nanoseconds)) {
    }
}

constexpr static int read_iterations = 1000;

BENCHMARK_TEMPLATE_DEFINE_F(WBRCUFixture, WBRCU_ProtectInt_Reader, uint64_t)(benchmark::State& state) {
    int numa_node = state.thread_index() % numa_num_configured_nodes();  
    bind_thread_to_node(numa_node);
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
    int numa_node = state.thread_index() % numa_num_configured_nodes();  
    bind_thread_to_node(numa_node);
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
    int numa_node = state.thread_index() % numa_num_configured_nodes();  
    bind_thread_to_node(numa_node);
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
    int numa_node = state.thread_index() % numa_num_configured_nodes();  
    bind_thread_to_node(numa_node);
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
