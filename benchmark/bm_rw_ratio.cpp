#include <thread>
#include <vector>
#include <iostream>

#include "common.hpp"
#include "wbrcu/rcu_protected.hpp"
#include "benchmark/benchmark.h"

template <template<typename> class Protect>
class RatioFixture : public benchmark::Fixture {
public:
    Protect<uint64_t> p{new uint64_t{}};
};

constexpr static int read_iterations = 1000000;
constexpr static int write_iterations = 100;

void simulate_work(int nanoseconds) {
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::high_resolution_clock::now() - start < std::chrono::nanoseconds(nanoseconds)) {
    }
}

void bm_read(benchmark::State& state, auto& p) {
    auto numWriters = WBRCU_HARDWARE_CONCURRENCY - state.threads();
    std::vector<std::jthread> writers;
    if (state.thread_index() == 0) {
        for (int i = 0; i < numWriters; ++i) {
            writers.emplace_back([&](std::stop_token st) {
                while (!st.stop_requested()) {
                    p.update([](uint64_t* ptr) { ++(*ptr); simulate_work(100); });
                }
            });
        }
    }

    uint64_t read_ops = 0;
    for (auto _ : state) {
        uint64_t k;
        for (int i = 0; i < read_iterations; ++i) {
            auto ptr = p.get_ptr();
            benchmark::DoNotOptimize(k = *ptr);
            simulate_work(100);
        }
        read_ops += read_iterations;
    }

    if (state.thread_index() == 0) {
        for (int i = 0; i < numWriters; ++i) {
            writers[i].request_stop();
        }
        writers.clear();
    }
    state.counters["read_ops_per_thread"] = benchmark::Counter(read_ops, benchmark::Counter::kIsRate);;
}

void bm_write(benchmark::State& state, auto& p) {
    auto numReaders = WBRCU_HARDWARE_CONCURRENCY - state.threads();
    std::vector<std::jthread> readers;
    if (state.thread_index() == 0) {
        for (int i = 0; i < numReaders; ++i) {
            readers.emplace_back([&](std::stop_token st) {
                uint64_t k;
                while (!st.stop_requested()) {
                    auto ptr = p.get_ptr();
                    benchmark::DoNotOptimize(k = *ptr);
                    simulate_work(100);
                }
            });
        }
    }

    uint64_t write_ops = 0;
    for (auto _ : state) {
        for (int i = 0; i < write_iterations; ++i) {
            p.update([](uint64_t* ptr) { ++(*ptr); simulate_work(100); });
        }
        write_ops += write_iterations;
    }

    if (state.thread_index() == 0) {
        for (int i = 0; i < numReaders; ++i) {
            readers[i].request_stop();
        }
        readers.clear();
    }
    state.counters["write_ops_per_thread"] = benchmark::Counter(write_ops, benchmark::Counter::kIsRate);;
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, WBRCU_Reader, wbrcu::rcu_protected)(benchmark::State& state) {
    bm_read(state, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, FollyRCU_Reader, follyrcu_protected)(benchmark::State& state) {
    bm_read(state, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, SharedMutex_Reader, rwlock_protected)(benchmark::State& state) {
    bm_read(state, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, Mutex_Reader, lock_protected)(benchmark::State& state) {
    bm_read(state, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, WBRCU_Writer, wbrcu::rcu_protected)(benchmark::State& state) {
    bm_write(state, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, FollyRCU_Writer, follyrcu_protected)(benchmark::State& state) {
    bm_write(state, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, SharedMutex_Writer, rwlock_protected)(benchmark::State& state) {
    bm_write(state, p);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, Mutex_Writer, lock_protected)(benchmark::State& state) {
    bm_write(state, p);
}

BENCHMARK_REGISTER_F(RatioFixture, WBRCU_Reader)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY-1);
BENCHMARK_REGISTER_F(RatioFixture, FollyRCU_Reader)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY-1);
BENCHMARK_REGISTER_F(RatioFixture, SharedMutex_Reader)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY-1);
BENCHMARK_REGISTER_F(RatioFixture, Mutex_Reader)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY-1);

BENCHMARK_REGISTER_F(RatioFixture, WBRCU_Writer)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY-1);
BENCHMARK_REGISTER_F(RatioFixture, FollyRCU_Writer)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY-1);
BENCHMARK_REGISTER_F(RatioFixture, SharedMutex_Writer)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY-1);
BENCHMARK_REGISTER_F(RatioFixture, Mutex_Writer)->ThreadRange(1, WBRCU_HARDWARE_CONCURRENCY-1);
