#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "wbrcu/rcu_protected.hpp"

class RCUTest : public ::testing::Test {
protected:
    struct TestObject {
        int value;
        explicit TestObject(int v) : value(v) {}
    };

    wbrcu::rcu_protected<TestObject> rcu_obj{new TestObject(0)};
};

TEST_F(RCUTest, InitialValueIsZero) {
    auto ptr = rcu_obj.get_ptr();
    EXPECT_EQ(ptr->value, 0);
}

TEST_F(RCUTest, BasicUpdate) {
    rcu_obj.update([](TestObject* obj) { obj->value = 42; });
    auto ptr = rcu_obj.get_ptr();
    EXPECT_EQ(ptr->value, 42);
}

TEST_F(RCUTest, MultithreadedReads) {
    constexpr int num_threads = 10;
    constexpr int num_reads = 1000;

    std::vector<std::thread> threads;
    std::atomic<int> successful_reads(0);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, &successful_reads]() {
            for (int j = 0; j < num_reads; ++j) {
                auto ptr = rcu_obj.get_ptr();
                if (ptr->value >= 0) {
                    successful_reads.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(successful_reads.load(), num_threads * num_reads);
}

TEST_F(RCUTest, ConcurrentReadsAndUpdates) {
    constexpr int num_reader_threads = 5;
    constexpr int num_updater_threads = 3;
    constexpr int num_operations = 1000;

    std::vector<std::thread> reader_threads;
    std::vector<std::thread> updater_threads;
    std::atomic<int> successful_reads(0);
    std::atomic<int> successful_updates(0);

    for (int i = 0; i < num_reader_threads; ++i) {
        reader_threads.emplace_back([this, &successful_reads]() {
            for (int j = 0; j < num_operations; ++j) {
                auto ptr = rcu_obj.get_ptr();
                if (ptr->value >= 0) {
                    successful_reads.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    for (int i = 0; i < num_updater_threads; ++i) {
        updater_threads.emplace_back([this, &successful_updates]() {
            for (int j = 0; j < num_operations; ++j) {
                rcu_obj.update([](TestObject* obj) {
                    obj->value++;
                });
                successful_updates.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : reader_threads) {
        t.join();
    }
    for (auto& t : updater_threads) {
        t.join();
    }

    EXPECT_EQ(successful_reads.load(), num_reader_threads * num_operations);
    EXPECT_EQ(successful_updates.load(), num_updater_threads * num_operations);

    auto final_ptr = rcu_obj.get_ptr();
    EXPECT_EQ(final_ptr->value, num_updater_threads * num_operations);
}

TEST_F(RCUTest, NestedUpdates) {
    rcu_obj.update([this](TestObject* obj) {
        obj->value = 1;
        rcu_obj.update([](TestObject* inner_obj) {
            inner_obj->value *= 2;
        });
    });

    auto ptr = rcu_obj.get_ptr();
    EXPECT_EQ(ptr->value, 2);
}

TEST_F(RCUTest, LargeNumberOfUpdates) {
    constexpr int num_updates = 10000;

    for (int i = 0; i < num_updates; ++i) {
        rcu_obj.update([i](TestObject* obj) {
            obj->value = i;
        });
    }

    auto ptr = rcu_obj.get_ptr();
    EXPECT_EQ(ptr->value, num_updates - 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}