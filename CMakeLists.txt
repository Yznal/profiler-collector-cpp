cmake_minimum_required (VERSION 3.14)
project(ap-collector)

set(CMAKE_CXX_STAN)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wextra-semi -O3 -g -fsanitize=address -fno-omit-frame-pointer")


add_executable(main src/main.cpp)
add_executable(producer src/producer.cpp)
add_executable(collector src/collector.cpp)

