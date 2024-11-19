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


constexpr static int read_iterations = 1000000;
constexpr static int write_iterations = 100;

BENCHMARK_TEMPLATE_DEFINE_F(WBRCUFixture, WBRCU_ProtectInt_Reader, uint64_t)(benchmark::State& state) {
    uint64_t read_ops = 0;
    for (auto _ : state) {
        int k;
        for (int i = 0; i < read_iterations; ++i) {
            auto ptr = p.get_ptr();
            benchmark::DoNotOptimize(k = *ptr);
        }
        read_ops += read_iterations;
    }
    state.counters["total_read_ops"] = benchmark::Counter(read_ops * state.threads(), benchmark::Counter::kIsRate);;
}

BENCHMARK_TEMPLATE_DEFINE_F(FollyRCUFixture, FollyRCU_ProtectInt_Reader, uint64_t)(benchmark::State& state) {
    uint64_t read_ops = 0;
    for (auto _ : state) {
        int k;
        for (int i = 0; i < read_iterations; ++i) {
            auto ptr = p.get_ptr();
            benchmark::DoNotOptimize(k = *ptr);
        }
        read_ops += read_iterations;
    }
    state.counters["total_read_ops"] = benchmark::Counter(read_ops * state.threads(), benchmark::Counter::kIsRate);;
}

BENCHMARK_TEMPLATE_DEFINE_F(SharedMutexFixture, SharedMutex_ProtectInt_Reader, uint64_t)(benchmark::State& state) {
    uint64_t read_ops = 0;
    for (auto _ : state) {
        int k;
        for (int i = 0; i < read_iterations; ++i) {
            auto ptr = p.get_ptr();
            benchmark::DoNotOptimize(k = *ptr);
        }
        read_ops += read_iterations;
    }
    state.counters["total_read_ops"] = benchmark::Counter(read_ops * state.threads(), benchmark::Counter::kIsRate);;
}

BENCHMARK_TEMPLATE_DEFINE_F(MutexFixture, Mutex_ProtectInt_Reader, uint64_t)(benchmark::State& state) {
    uint64_t read_ops = 0;
    for (auto _ : state) {
        int k;
        for (int i = 0; i < read_iterations; ++i) {
            auto ptr = p.get_ptr();
            benchmark::DoNotOptimize(k = *ptr);
        }
        read_ops += read_iterations;
    }
    state.counters["total_read_ops"] = benchmark::Counter(read_ops * state.threads(), benchmark::Counter::kIsRate);;
}

BENCHMARK_TEMPLATE_DEFINE_F(WBRCUFixture, WBRCU_ProtectInt_Writer, uint64_t)(benchmark::State& state) {
    uint64_t write_ops = 0;
    for (auto _ : state) {
        for (int i = 0; i < write_iterations; ++i) {
            p.update([](uint64_t* ptr) { ++(*ptr); });
        }
        write_ops += write_iterations;
    }
    state.counters["total_write_ops"] = benchmark::Counter(write_ops * state.threads(), benchmark::Counter::kIsRate);;
}

BENCHMARK_TEMPLATE_DEFINE_F(FollyRCUFixture, FollyRCU_ProtectInt_Writer, uint64_t)(benchmark::State& state) {
    uint64_t write_ops = 0;
    for (auto _ : state) {
        for (int i = 0; i < write_iterations; ++i) {
            p.update([](uint64_t* ptr) { ++(*ptr); });
        }
        write_ops += write_iterations;
    }
    state.counters["total_write_ops"] = benchmark::Counter(write_ops * state.threads(), benchmark::Counter::kIsRate);;
}

BENCHMARK_TEMPLATE_DEFINE_F(SharedMutexFixture, SharedMutex_ProtectInt_Writer, uint64_t)(benchmark::State& state) {
    uint64_t write_ops = 0;
    for (auto _ : state) {
        for (int i = 0; i < write_iterations; ++i) {
            p.update([](uint64_t* ptr) { ++(*ptr); });
        }
        write_ops += write_iterations;
    }
    state.counters["total_write_ops"] = benchmark::Counter(write_ops * state.threads(), benchmark::Counter::kIsRate);;
}

BENCHMARK_TEMPLATE_DEFINE_F(MutexFixture, Mutex_ProtectInt_Writer, uint64_t)(benchmark::State& state) {
    uint64_t write_ops = 0;
    for (auto _ : state) {
        for (int i = 0; i < write_iterations; ++i) {
            p.update([](uint64_t* ptr) { ++(*ptr); });
        }
        write_ops += write_iterations;
    }
    state.counters["total_write_ops"] = benchmark::Counter(write_ops * state.threads(), benchmark::Counter::kIsRate);;
}


BENCHMARK_REGISTER_F(WBRCUFixture, WBRCU_ProtectInt_Reader)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(FollyRCUFixture, FollyRCU_ProtectInt_Reader)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(SharedMutexFixture, SharedMutex_ProtectInt_Reader)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(MutexFixture, Mutex_ProtectInt_Reader)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY);

BENCHMARK_REGISTER_F(WBRCUFixture, WBRCU_ProtectInt_Writer)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(FollyRCUFixture, FollyRCU_ProtectInt_Writer)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(SharedMutexFixture, SharedMutex_ProtectInt_Writer)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(MutexFixture, Mutex_ProtectInt_Writer)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY);
