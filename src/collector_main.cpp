#include <csignal>
#include <chrono>
#include <thread>
#include "prometheus/prometheus_store.hpp"


using namespace yznal::trace_collector;

int main(int argc, char** argv) {
    
    prometheus_store storage;

    std::this_thread::sleep_for(std::chrono::seconds(30));
    
    return 0;
}


