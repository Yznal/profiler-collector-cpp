#pragma once

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace yznal::trace_collector {


    class prometheus_store {
    public:

        prometheus_store();

        void record_sample(const std::string& caller, const std::string& callee, size_t samples_count);

    private:
        std::shared_ptr<prometheus::Registry> registry_;
        std::function<prometheus::Counter*(const std::string&, const std::string&)> counter_factory_;
        prometheus::Exposer exposer_;
        std::unordered_map<std::string, std::unordered_map<std::string, prometheus::Counter*>> counters_;

    };
    
}
