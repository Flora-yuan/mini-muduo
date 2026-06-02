cmake_minimum_required(VERSION 3.10)

project(mini_muduo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(mini_muduo STATIC
    src/base/Logger.cc
    src/base/Timestamp.cc
    src/net/Channel.cc
    src/net/EPollPoller.cc
    src/net/InetAddress.cc
    src/net/Poller.cc
    src/net/SocketsOps.cc
    src/net/Socket.cc
)

add_executable(example_basic
    examples/example_basic.cc
)
target_link_libraries(example_basic mini_muduo)
set_target_properties(example_basic PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/examples
)

add_executable(test_timestamp
    tests/test_timestamp.cc
)
target_link_libraries(test_timestamp mini_muduo)
set_target_properties(test_timestamp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests
)

add_executable(test_socket_wrappers
    tests/test_socket_wrappers.cc
)
target_link_libraries(test_socket_wrappers mini_muduo)
set_target_properties(test_socket_wrappers PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests
)

add_executable(test_epoll_poller
    tests/test_epoll_poller.cc
)
target_link_libraries(test_epoll_poller mini_muduo)
set_target_properties(test_epoll_poller PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests
)

add_executable(blocking_echo_server
    examples/blocking_echo_server.cc
)
target_link_libraries(blocking_echo_server mini_muduo)
set_target_properties(blocking_echo_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/examples
)
