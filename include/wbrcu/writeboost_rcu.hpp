#pragma once

#include <iostream>

namespace wbrcu {

class rcu_domain {
public:
    void foo() {
        std::cout << "foo\n";
    }
};

}