
#include <mutex>
#include <concepts>

#include "wbrcu/writeboost_rcu.hpp"
#include "folly/synchronization/Rcu.h"

int main() {
    wbrcu::rcu_domain d;
    d.foo();

    std::scoped_lock guard(folly::rcu_default_domain());
}