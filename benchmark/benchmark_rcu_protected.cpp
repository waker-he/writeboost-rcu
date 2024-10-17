#include <shared_mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <iostream>

#include "wbrcu/rcu_protected.hpp"
#include "folly/synchronization/Rcu.h"
#include "benchmark/benchmark.h"

template <class ProtectedType>
class WBRCUFixture : public benchmark::Fixture {
public:
    wbrcu::rcu_protected<ProtectedType> p{new ProtectedType{}};
};

template <class ProtectedType>
class SharedMutexFixture : public benchmark::Fixture {
public:
    std::shared_mutex mut;
    ProtectedType p{};
};

template <class ProtectedType>
class FollyRCUFixture : public benchmark::Fixture {
public:
    std::mutex mut;
    std::atomic<ProtectedType*> ptr{new ProtectedType{}};
};

constexpr static int read_iterations = 1000000;
constexpr static int write_iterations = 1000;

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
            std::lock_guard lck{folly::rcu_default_domain()};
            benchmark::DoNotOptimize(k = *ptr.load(std::memory_order_acquire));
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
            std::shared_lock lck{mut};
            benchmark::DoNotOptimize(k = p);
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
            mut.lock();
            auto newPtr = new uint64_t{*ptr.load(std::memory_order_relaxed)};
            ++(*newPtr);
            auto oldPtr = ptr.exchange(newPtr, std::memory_order_release);
            mut.unlock();
            folly::rcu_retire(oldPtr);
        }
        write_ops += write_iterations;
    }
    state.counters["total_write_ops"] = benchmark::Counter(write_ops * state.threads(), benchmark::Counter::kIsRate);;
}

BENCHMARK_TEMPLATE_DEFINE_F(SharedMutexFixture, SharedMutex_ProtectInt_Writer, uint64_t)(benchmark::State& state) {
    uint64_t write_ops = 0;
    for (auto _ : state) {
        for (int i = 0; i < write_iterations; ++i) {
            std::lock_guard lck{mut};
            ++p;
        }
        write_ops += write_iterations;
    }
    state.counters["total_write_ops"] = benchmark::Counter(write_ops * state.threads(), benchmark::Counter::kIsRate);;
}


BENCHMARK_REGISTER_F(WBRCUFixture, WBRCU_ProtectInt_Reader)->ThreadRange(1, 8);
BENCHMARK_REGISTER_F(FollyRCUFixture, FollyRCU_ProtectInt_Reader)->ThreadRange(1, 8);
BENCHMARK_REGISTER_F(SharedMutexFixture, SharedMutex_ProtectInt_Reader)->ThreadRange(1,8);

BENCHMARK_REGISTER_F(WBRCUFixture, WBRCU_ProtectInt_Writer)->ThreadRange(1,8);
BENCHMARK_REGISTER_F(FollyRCUFixture, FollyRCU_ProtectInt_Writer)->ThreadRange(1,8);
BENCHMARK_REGISTER_F(SharedMutexFixture, SharedMutex_ProtectInt_Writer)->ThreadRange(1,8);
