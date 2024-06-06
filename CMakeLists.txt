cmake_minimum_required (VERSION 3.14)
project(ap-collector)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wextra-semi -O3 -g -fsanitize=address -fno-omit-frame-pointer")


add_executable(stdin_main src/stdin_main.cpp src/parser/trace_parser.cpp src/reader.cpp src/stack_trie.cpp)
add_executable(producer src/producer.cpp)
add_executable(collector src/collector.cpp)

