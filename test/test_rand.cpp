#include "wbrcu/rcu_protected.hpp"

constexpr std::array<uint64_t, 100> rands = {
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    // 20
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    // 40
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    // 60
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    // 80
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(), wbrcu::rand(),
    // 100
};

consteval bool has_duplicates() {
    for (int i = 0; i < 100; ++i) {
        for (int j = i+1; j < 100; ++j) {
            if (rands[i] == rands[j]) return true;
        }
    }
    return false;
}

static_assert(!has_duplicates());

int main() {}