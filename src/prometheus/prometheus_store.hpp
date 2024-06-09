#pragma once

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace yznal::trace_collector {


    class counter {
    public:

        counter(std::shared_ptr<prometheus::Counter> cnt_ptr);

        void increment(size_t value) ;


    private:
        std::weak_ptr<prometheus::Counter> cnt_;
    };

    class prometheus_store {
    public:

        prometheus_store();

        counter get_or_create_counter(const std::string& caller, const std::string& callee);

    private:
        std::shared_ptr<prometheus::Registry> registry_;
        prometheus::Exposer exposer_;
        std::function<prometheus::Counter*(const std::string&, const std::string&)> counter_factory_;
        std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<prometheus::Counter>>> counters_;

    };
    
}
