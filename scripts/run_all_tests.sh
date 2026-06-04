#!/usr/bin/env bash
set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

rm -rf build
mkdir build
cd build

cmake ..
make -j$(nproc)

./tests/test_timestamp
./tests/test_socket_wrappers
./tests/test_epoll_poller
./tests/test_event_loop
./tests/test_acceptor
./tests/test_buffer
./tests/test_tcp_connection
./tests/test_tcp_server
./tests/test_event_loop_thread
./tests/test_event_loop_thread_pool
./tests/test_timer_queue
./tests/test_echo_server_integration

echo "all tests passed"
