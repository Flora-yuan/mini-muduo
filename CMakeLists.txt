cmake_minimum_required(VERSION 3.10)

project(mini_muduo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(mini_muduo STATIC
    src/base/Logger.cc
    src/base/Timestamp.cc
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
