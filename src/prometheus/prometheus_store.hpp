#pragma once

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include <string>
#include <memory>
#include <functional>

namespace yznal::trace_collector {


    class metric {
    public:
        metric(prometheus::Counter* cnt_ptr);
        metric();

        metric(const metric&) = default;
        metric(metric&&) = default;

        void increment(size_t value);
        void reset();


    private:
        prometheus::Counter* cnt_;
    };

    class prometheus_store {
    public:

        prometheus_store();

        metric create_counter(const std::string& caller, const std::string& callee, int level);

    private:
        std::shared_ptr<prometheus::Registry> registry_;
        prometheus::Exposer exposer_;
        std::function<prometheus::Counter*(const prometheus::Labels&)> Counter_factory_;

    };
    
}
