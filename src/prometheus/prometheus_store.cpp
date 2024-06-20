#include "prometheus_store.hpp"


namespace yznal::trace_collector {

    metric::metric(prometheus::Counter* cnt_ptr) : cnt_(cnt_ptr) { }
    metric:: metric() : metric(nullptr) { }

    void metric::increment(size_t value) {
        if (cnt_) 
            cnt_->Increment(value);
    }

    void metric::reset() {
        if (cnt_) 
            cnt_->Reset();
    }


    prometheus_store::prometheus_store() : 
    registry_(new prometheus::Registry()), 
    exposer_("172.17.0.1:8082"), 
    Counter_factory_(
        [&metric_family = prometheus::BuildCounter().Name("profiler_samples").Help("async-profiler method samples").Register(*registry_)]
        (const prometheus::Labels &labels) {
            return &metric_family.Add(labels);
        }
    ) {
        exposer_.RegisterCollectable(registry_, "/metrics");
    }

    // TODO: refactor so much, remove caching

    metric prometheus_store::create_counter(const std::string& caller, const std::string& callee, int level) {
        return metric { Counter_factory_({{"caller", caller}, {"callee", callee}, {"depth", std::to_string(level)}}) };
    }

    metric prometheus_store::get_or_create_counter(const std::string& caller, const std::string& callee) {
        if (!counters_.contains(caller) || !counters_[caller].contains(callee)) {
            counters_[caller][callee] = Counter_factory_({{"caller", caller}, {"callee", callee}});
        }
        return metric {counters_[caller][callee]};
    }

    
}


