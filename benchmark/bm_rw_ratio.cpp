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





// New: Explicit Ratio Test
template <template<typename> class Protect>
void bm_explicit_ratio(benchmark::State& state, Protect<uint64_t>& p, int read_ratio) {
    // Number of readers and writers based on the ratio
    int total_threads = state.threads();
    int num_readers = total_threads * read_ratio / 100;
    int num_writers = total_threads - num_readers;

    // Create writer threads
    std::vector<std::jthread> writers;
    if (state.thread_index() == 0) {
        for (int i = 0; i < num_writers; ++i) {
            writers.emplace_back([&](std::stop_token st) {
                while (!st.stop_requested()) {
                    p.update([](uint64_t* ptr) { ++(*ptr); simulate_work(100); });
                }
            });
        }
    }

    uint64_t ops = 0;
    for (auto _ : state) {
        if (state.thread_index() < num_readers) {
            // Reader logic
            for (int i = 0; i < read_iterations; ++i) {
                auto ptr = p.get_ptr();
                benchmark::DoNotOptimize(*ptr);
                simulate_work(100);
            }
            ops += read_iterations;
        } else {
            // Writer logic
            for (int i = 0; i < write_iterations; ++i) {
                p.update([](uint64_t* ptr) { ++(*ptr); simulate_work(100); });
            }
            ops += write_iterations;
        }
    }

    if (state.thread_index() == 0) {
        for (auto& writer : writers) {
            writer.request_stop();
        }
        writers.clear();
    }
    state.counters["ops_per_thread"] = benchmark::Counter(ops, benchmark::Counter::kIsRate);
}

// Register explicit ratio tests for multiple ratios
BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, WBRCU_80_20, wbrcu::rcu_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/80); // 80% readers, 20% writers
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, WBRCU_90_10, wbrcu::rcu_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/90); // 90% readers, 10% writers
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, WBRCU_60_40, wbrcu::rcu_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/60); // 60% readers, 40% writers
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, WBRCU_40_60, wbrcu::rcu_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/40); // 40% readers, 60% writers
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, WBRCU_20_80, wbrcu::rcu_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/20); // 20% readers, 80% writers
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, WBRCU_10_90, wbrcu::rcu_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/10); // 10% readers, 90% writers
}

// Register the same for FollyRCU
BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, FollyRCU_80_20, follyrcu_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/80);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, FollyRCU_90_10, follyrcu_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/90);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, FollyRCU_60_40, follyrcu_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/60);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, FollyRCU_40_60, follyrcu_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/40);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, FollyRCU_20_80, follyrcu_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/20);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, FollyRCU_10_90, follyrcu_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/10);
}

// Register the same for SharedMutex
BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, SharedMutex_80_20, rwlock_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/80);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, SharedMutex_90_10, rwlock_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/90);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, SharedMutex_60_40, rwlock_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/60);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, SharedMutex_40_60, rwlock_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/40);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, SharedMutex_20_80, rwlock_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/20);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, SharedMutex_10_90, rwlock_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/10);
}

// Register the same for Mutex
BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, Mutex_80_20, lock_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/80);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, Mutex_90_10, lock_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/90);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, Mutex_60_40, lock_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/60);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, Mutex_40_60, lock_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/40);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, Mutex_20_80, lock_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/20);
}

BENCHMARK_TEMPLATE_DEFINE_F(RatioFixture, Mutex_10_90, lock_protected)
(benchmark::State& state) {
    bm_explicit_ratio(state, p, /*read_ratio=*/10);
}

// Register all ratio tests for thread ranges
BENCHMARK_REGISTER_F(RatioFixture, WBRCU_80_20)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, WBRCU_90_10)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, WBRCU_60_40)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, WBRCU_40_60)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, WBRCU_20_80)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, WBRCU_10_90)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);

BENCHMARK_REGISTER_F(RatioFixture, FollyRCU_80_20)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, FollyRCU_90_10)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, FollyRCU_60_40)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, FollyRCU_40_60)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, FollyRCU_20_80)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, FollyRCU_10_90)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);

BENCHMARK_REGISTER_F(RatioFixture, SharedMutex_80_20)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, SharedMutex_90_10)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, SharedMutex_60_40)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, SharedMutex_40_60)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, SharedMutex_20_80)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, SharedMutex_10_90)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);

BENCHMARK_REGISTER_F(RatioFixture, Mutex_80_20)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, Mutex_90_10)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, Mutex_60_40)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, Mutex_40_60)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, Mutex_20_80)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);
BENCHMARK_REGISTER_F(RatioFixture, Mutex_10_90)->ThreadRange(2, WBRCU_HARDWARE_CONCURRENCY);

