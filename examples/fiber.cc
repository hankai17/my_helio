//
// Created by root on 1/17/23.
//


#include <iostream>
#include <boost/fiber/all.hpp>

struct callable {
    void operator()() {
        std::cout << "callable" << std::endl;
    };
};

class A {
public:
    A() {
        std::cout << "construct A" << std::endl;
    }
    ~A() {
        std::cout << "deconstruct A" << std::endl;
    }
};

boost::fibers::fiber copies_are_safe() {
    callable x;
    return boost::fibers::fiber(x);
} // x is destroyed, but the newly-created fiber has a copy, so this is OK

boost::fibers::fiber oops() {
    callable x;
    return boost::fibers::fiber( std::ref(x) );
} // x is destroyed, but the newly-created fiber still has a reference
// this leads to undefined behaviour

int main1() {
    std::cout << "Hello, World!" << std::endl;
    auto f = copies_are_safe(); // 1. fiber挂到schedule队列里
    sleep(3);
    f.join(); // 2. 从队列拿一个resume
    return 0;
}

int main() {
    std::cout << "before fiber, active id: " << boost::fibers::context::active()->get_id() << std::endl;
    auto f = copies_are_safe(); // 1. fiber挂到schedule队列里
    std::cout << &f << ", id: " << f.get_id() << std::endl;
    f.detach(); //impl析构 那么impl只存在于schedule队列里
    std::cout << "after fiber detach, active id: " << boost::fibers::context::active()->get_id() << std::endl;

    //boost::fibers::context::active()->resume(); // resume current active // coredown
    //std::cout << "has_ready_fibers: " << boost::fibers::has_ready_fibers() << std::endl;
    //std::cout << boost::fibers::context::active()->get_scheduler()->has_ready_fibers() << std::endl;
    boost::fibers::context::active()->get_scheduler()->suspend();
    A a;
    return 0; // 因为schedule队列没有调用时机 那么在析构时会遍历队列依次resume
}
