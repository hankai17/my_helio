cmake_minimum_required(VERSION 3.13)
project(helio)

set(CMAKE_C_COMPILER /usr/local//gcc/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/local/gcc/bin/g++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")

include_directories(./)
include_directories(/usr/local/include/mimalloc-2.0/)
link_directories(/root/file/gcc-9.1.0/x86_64-pc-linux-gnu/libstdc++-v3/src/.libs)
link_directories(/root/file/boost_1_72_0/stage/lib)

find_package(absl REQUIRED)
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

set(LIBS ${PROJECT_NAME}
        xxhash
        uring
        absl
        profiler
        mimalloc
        glog
        boost_system
        boost_fiber
        boost_context
        boost_filesystem
        anl
        jemalloc
        pthread
        ${OPENSSL_LIBRARIES}
        )

set(LIB_SRC
        base/pmr/arena.cc
        base/pmr/arena.h
        base/pmr/pod_array.h
        base/aquahash.h
        base/bits.h
        base/cuckoo_map-internal.h
        base/cuckoo_map.cc
        base/cuckoo_map.h
        base/endian.h
        base/expected.hpp
        base/flags.h
        base/flit.h
        base/function2.hpp
        base/gtest.h
        #base/gtest_main.cc
        base/hash.cc
        base/hash.h
        base/histogram.cc
        base/histogram.h
        base/init.cc
        base/init.h
        base/integral_types.h
        base/io_buf.h
        base/libdivide.h
        base/logging.cc
        base/logging.h
        base/mpmc_bounded_queue.h
        base/pod_array.h
        base/port.h
        base/proc_util.cc
        base/proc_util.h
        base/ProducerConsumerQueue.h
        base/pthread_utils.cc
        base/pthread_utils.h
        base/random.h
        base/ring_buffer.h
        base/RWSpinLock.h
        base/sse2neon.h
        base/stl_util.h
        base/string_view_sso.h
        base/type_traits.h
        base/varz_node.cc
        base/varz_node.h
        base/varz_value.h
        base/zipf_gen.h
        io/file.cc
        io/file.h
        io/file_util.cc
        io/file_util.h
        io/io.cc
        io/io.h
        io/line_reader.cc
        io/line_reader.h
        io/proc_reader.cc
        io/proc_reader.h
        strings/human_readable.cc
        strings/human_readable.h
        #util/cloud/aws.cc
        #util/cloud/aws.h
        #util/cloud/s3.cc
        #util/cloud/s3.h
        util/epoll/epoll_fiber_scheduler.cc
        util/epoll/epoll_fiber_scheduler.h
        util/epoll/epoll_pool.cc
        util/epoll/epoll_pool.h
        util/epoll/epoll_socket.cc
        util/epoll/epoll_socket.h
        util/epoll/proactor.cc
        util/epoll/proactor.h
        util/fibers/event_count.h
        util/fibers/fiber_file.cc
        util/fibers/fiber_file.h
        util/fibers/fiberqueue_threadpool.cc
        util/fibers/fiberqueue_threadpool.h
        util/fibers/fibers_ext.cc
        util/fibers/fibers_ext.h
        util/fibers/simple_channel.h
        util/html/sorted_table.cc
        util/html/sorted_table.h
        util/http/beast_rj_utils.h
        util/http/encoding.cc
        util/http/encoding.h
        util/http/http_client.cc
        util/http/http_client.h
        util/http/http_common.cc
        util/http/http_common.h
        util/http/http_handler.cc
        util/http/http_handler.h
        #util/http/http_main.cc
        util/http/http_status_code.cc
        util/http/http_status_code.h
        #util/http/http_testing.cc
        #util/http/http_testing.h
        #util/http/https_client.cc
        #util/http/https_client.h
        #util/http/https_client_pool.cc
        #util/http/https_client_pool.h
        util/http/prebuilt_beast.cc
        util/http/profilez_handler.cc
        util/http/status_page.cc
        util/metrics/family.cc
        util/metrics/family.h
        util/metrics/metrics.cc
        util/metrics/metrics.h
        util/tls/tls_engine.cc
        util/tls/tls_engine.h
        util/tls/tls_socket.cc
        util/tls/tls_socket.h
        util/uring/proactor.cc
        util/uring/proactor.h
        util/uring/submit_entry.h
        util/uring/uring_fiber_algo.cc
        util/uring/uring_fiber_algo.h
        util/uring/uring_file.cc
        util/uring/uring_file.h
        util/uring/uring_pool.cc
        util/uring/uring_pool.h
        util/uring/uring_socket.cc
        util/uring/uring_socket.h
        util/accept_server.cc
        util/accept_server.h
        util/asio_stream_adapter.h
        util/connection.h
        util/dns_resolve.cc
        util/dns_resolve.h
        util/fiber_sched_algo.cc
        util/fiber_sched_algo.h
        util/fiber_socket_base.cc
        util/fiber_socket_base.h
        util/listener_interface.cc
        util/listener_interface.h
        util/prebuilt_asio.cc
        util/proactor_base.cc
        util/proactor_base.h
        util/proactor_pool.cc
        util/proactor_pool.h
        util/sliding_counter.cc
        util/sliding_counter.h
        util/varz.cc
        util/varz.h)

add_library(${PROJECT_NAME} SHARED ${LIB_SRC})

function(helio_add_executable targetname srcs depends libs)
    add_executable(${targetname} ${srcs})
    add_dependencies(${targetname} ${depends})
    #force_redefine_file_macro_for_source(${targetname})
    target_link_libraries(${targetname} ${libs})
endfunction()

helio_add_executable(fiber examples/fiber.cc  ${PROJECT_NAME} "${LIBS}")
helio_add_executable(echo_server examples/echo_server.cc  ${PROJECT_NAME} "${LIBS}")
