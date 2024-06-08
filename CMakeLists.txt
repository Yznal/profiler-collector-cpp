cmake_minimum_required (VERSION 3.14)
project(ap-collector)

find_package(prometheus-cpp CONFIG REQUIRED)

if(NOT DEFINED prometheus-cpp_VERSION)
  message(FATAL_ERROR "prometheus-cpp_VERSION is not defined")
endif()

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wextra-semi -O3 -g -fsanitize=address -fno-omit-frame-pointer")

file(GLOB core src/core/**.cpp)
add_executable(stdin_main src/stdin_main.cpp ${core})


target_link_libraries(stdin_main PRIVATE prometheus-cpp::pull)
