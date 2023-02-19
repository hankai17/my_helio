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
    auto f = copies_are_safe(); // 1. fiber挂到schedule队列里
    sleep(3);
    f.join(); // 2. 从队列拿一个resume
    std::cout << "Hello, World!" << std::endl; // 3. join消费完毕会走到这里
    return 0;
}

int main2() {
    std::cout << "before fiber, active id: " << boost::fibers::context::active()->get_id() << std::endl;
    auto f = copies_are_safe(); // 1. fiber挂到schedule队列里
    std::cout << &f << ", id: " << f.get_id() << std::endl;
    f.detach(); //impl析构 那么impl只存在于schedule队列里
    std::cout << "after fiber detach, active id: " << boost::fibers::context::active()->get_id() << std::endl;
    A a;
    return 0; // 因为schedule队列没有调用时机 那么在析构时会遍历队列依次resume
}

int main3() {
    std::cout << "before fiber, active id: " << boost::fibers::context::active()->get_id() << std::endl;
    auto f = copies_are_safe(); // 1. fiber挂到schedule队列里
    std::cout << &f << ", id: " << f.get_id() << std::endl;
    f.detach(); //impl析构 那么impl只存在于schedule队列里
    std::cout << "after fiber detach, active id: " << boost::fibers::context::active()->get_id() << std::endl;
    std::cout << "has_ready_fibers: " << boost::fibers::has_ready_fibers() << std::endl; // 队列上确实有ready fiber
    std::cout << boost::fibers::context::active()->get_scheduler()->has_ready_fibers() << std::endl; // 队列上确实有ready fiber
    // 如何主动让队列消费fiber？
    // 失败方案1
    boost::fibers::context::active()->get_scheduler()->suspend(); // 即手动调用让队列pick_next消费之
    // 但fiber运行完 就直接卡到suspend里了
    // 失败方案2
    //boost::fibers::context::active()->resume(); // resume current active // coredown
    A a;
    return 0;
}

using ready_queue_type = ::boost::fibers::scheduler::ready_queue_type;
ready_queue_type rqueue_;

int main() {
#if 1
    auto main_cntx = boost::fibers::context::active();
    auto main_scheduler = boost::fibers::context::active()->get_scheduler();
    std::cout << "active is main_ctx? " << main_cntx->is_context(boost::fibers::type::main_context) << std::endl; // 1
    std::cout << "id: " << main_cntx->get_id() << std::endl;
    while (1) {
        sleep(1);
        // do something to push_back fiber into ready_list
        auto f = copies_are_safe();
        f.detach();
        //main_cntx_->ready_link(rqueue_);
        //main_scheduler->schedule(main_cntx);
        //main_scheduler->suspend();
        boost::this_fiber::yield();
        //->is_context(fibers::type::dispatcher_context));
        std::cout << "after suspend" << std::endl;
    }
#else
    auto worker_ctx = boost::fibers::worker_context();
#endif
    return 0;
}

void thread(std::uint32_t thread_count)
{
    boost::fibers::use_scheduling_algorithm<boost::fibers::algo::work_stealing>(thread_count);

    while (1) {
        auto f = copies_are_safe();
        f.detach();
        sleep(1);
    }
}

int main5()
{
    std::uint32_t thread_count = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    for (std::uint32_t i = 1; i < thread_count; i++) {
        threads.emplace_back(thread, thread_count);
    }
    boost::fibers::use_scheduling_algorithm<boost::fibers::algo::work_stealing>(thread_count);
    while (1) {
        std::cout << "cycling..." << std::endl;
        sleep(1);
    }
    return 0;
}

#if 0
void normal_fun(boost::context::detail::transfer_t transfer)
{
    std::cout << "normal_fun..."<< ", &transfer: " << &transfer
              << ": <transfer.fctx: " << transfer.fctx << ", transfer.data: " << transfer.data << ">" << std::endl; // 为什么跟make_fcontext返出得ctx不一样?
    auto ret_ctx = boost::context::detail::jump_fcontext(transfer.fctx, 0); // 2 from is 84a180, current fctx 12ff80
    std::cout << "normal_fun2..."<< ", &transfer: " << &ret_ctx
              << ": <ret_ctx.fctx: " << ret_ctx.fctx << ", ret_ctx.data: " << ret_ctx.data << ">" << std::endl; // 4 from is 84a180
    ret_ctx = boost::context::detail::jump_fcontext(ret_ctx.fctx, 0);

}

int transfer_fun_test()
{
    int stacksize = 1024 * 1024;
    char *stack = (char*)malloc(1024 * 1024);
    boost::context::detail::fcontext_t ctx  = boost::context::detail::make_fcontext(stack, stacksize, normal_fun);
    std::cout << "ctx: " << ctx << std::endl;
    auto ret_ctx = boost::context::detail::jump_fcontext(ctx, 0); // 1 current fctx 84a180
    std::cout << "normal_fun1..."<< ", &ret_ctx: " << &ret_ctx
              << ": <ret_ctx.fctx: " << ret_ctx.fctx << ", ret_ctx.data: " << ret_ctx.data << ">" << std::endl; // 3 from is 12ff80
    ret_ctx = boost::context::detail::jump_fcontext(ret_ctx.fctx, 0); // 3 from is 12ff80
    std::cout << "normal_fun3..."<< ", &ret_ctx: " << &ret_ctx
              << ": <ret_ctx.fctx: " << ret_ctx.fctx << ", ret_ctx.data: " << ret_ctx.data << ">" << std::endl; // 5 from is 12ff80
    return 0;
}
#else
#if 1 == 0
void normal_fun(boost::context::detail::transfer_t transfer)
{
    std::cout << "normal_fun..."<< ", transfer: " << &transfer
              << ": <transfer.fctx: " << transfer.fctx << ", transfer.data: " << transfer.data << ">" << std::endl;
    //boost::context::detail::jump_fcontext(transfer.fctx, 0);
}

boost::context::detail::transfer_t ontop_fun(boost::context::detail::transfer_t transfer)
{
    std::cout << "ontop_fun..." << ", transfer: " << &transfer
              << ": <transfer.fctx: " << transfer.fctx << ", transfer.data: " << transfer.data << ">" << std::endl;
    //auto ret_ctx = boost::context::detail::jump_fcontext(transfer.fctx, 0); // 提前返回
    return transfer;
}

int transfer_fun_test()
{
    int stacksize = 1024 * 1024;
    char *stack = (char*)malloc(1024 * 1024);
    boost::context::detail::fcontext_t ctx  = boost::context::detail::make_fcontext(stack, stacksize, normal_fun);
    std::cout << "ctx: " << ctx << std::endl;

    auto ret_ctx = boost::context::detail::ontop_fcontext(ctx, 0, ontop_fun);
    std::cout << "ontop_fun1..." << std::endl;
    return 0;
}
#elif 1 == 1
/*
    template< typename Ctx, typename Fn >
    transfer_t fiber_ontop( transfer_t t) {                     // 2.1 fiber_ontop函数即是对fn得封装而已
        auto p = *static_cast< Fn * >( t.data);
        t.data = nullptr;
        // execute function, pass fiber via reference
        Ctx c = p( Ctx{ t.fctx } );                             // 2.2 执行回调fn 传参为从何处来 即在那个fiber上触发的该fiber
        return { std::exchange( c.fctx_, nullptr), nullptr };
    }

    template< typename Fn >
    fiber resume_with( Fn && fn) && {
        return { detail::ontop_fcontext(
                    std::exchange(fctx_, nullptr),              // 1. 在fctx_上运行 fiber_ontop函数
                    &fn,
                    detail::fiber_ontop< fiber, decltype(fn) >  // 2. fiber_ontop函数即是对fn得封装而已
                 ).fctx 
        };
    }
    // resume_with的意思就是在(当前fiber即this上)fctx上运行fn
*/
boost::context::detail::transfer_t ontop_fun(boost::context::detail::transfer_t transfer) // ontop函数 见名知意 即就像在要运行的fiber之前插入了一段代码一样
{
    std::cout << "ontop_fun..." << ", transfer: " << &transfer
              << ": <transfer.fctx: " << transfer.fctx << ", transfer.data: " << transfer.data << ">" << std::endl;
    //boost::context::detail::jump_fcontext(transfer.fctx, 0); // 跳到normal_fun处 // 如果注掉则从main里直接退出
    return transfer;
}

void normal_fun(boost::context::detail::transfer_t transfer)
{
    std::cout << "normal_fun..."<< ", transfer: " << &transfer
              << ": <transfer.fctx: " << transfer.fctx << ", transfer.data: " << transfer.data << ">" << std::endl;
    auto ret_ctx = boost::context::detail::ontop_fcontext(transfer.fctx, 0, ontop_fun);
    std::cout << "normal_fun..." << std::endl;
    boost::context::detail::jump_fcontext(transfer.fctx, 0);
}

int transfer_fun_test() {
    int stacksize = 1024 * 1024;
    char *stack = (char *) malloc(1024 * 1024);
    boost::context::detail::fcontext_t ctx = boost::context::detail::make_fcontext(stack, stacksize, normal_fun);
    std::cout << "ctx: " << ctx << std::endl;
    boost::context::detail::jump_fcontext(ctx, 0);
    std::cout << "main_ctx_fun..." << std::endl;

    return 0;
}
#else
void normal_fun(boost::context::detail::transfer_t transfer)
{
    std::cout << "normal_fun..."<< ", transfer: " << &transfer
            << ": <transfer.fctx: " << transfer.fctx << ", traansfer.data: " << transfer.data << ">" << std::endl;
    boost::context::detail::jump_fcontext(transfer.fctx, 0);
}

boost::context::detail::transfer_t ontop_fun(boost::context::detail::transfer_t transfer)
{
    std::cout << "ontop_fun..." << ", transfer: " << &transfer
            << ": <transfer.fctx: " << transfer.fctx << ", transfer.data: " << transfer.data << ">" << std::endl;
    return boost::context::detail::transfer_t {transfer.fctx, 0};
}

int transfer_fun_test()
{
    int stacksize = 1024 * 1024;
    char *stack = (char *) malloc(1024 * 1024);
    boost::context::detail::fcontext_t ctx = boost::context::detail::make_fcontext(stack, stacksize, normal_fun);
    std::cout << "ctx: " << ctx << std::endl;

    auto ret_ctx = boost::context::detail::ontop_fcontext(ctx, 0, ontop_fun);
    std::cout << "ontop_fun1..." << std::endl;
    return 0;
}
#endif
#endif

int main0()
{
    transfer_fun_test();

    return 0;
}
