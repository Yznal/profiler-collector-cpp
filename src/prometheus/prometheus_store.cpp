#include "prometheus_store.hpp"



namespace yznal::trace_collector {

    counter::counter(std::shared_ptr<prometheus::Counter> cnt_ptr) : cnt_(cnt_ptr) { }

    void counter::increment(size_t value) {
        if (std::shared_ptr<prometheus::Counter> c_ptr = cnt_.lock()) c_ptr->Increment(value);
    }



    prometheus_store::prometheus_store() : 
    registry_(new prometheus::Registry()), 
    exposer_("localhost:8082"), 
    counter_factory_(
        [&metric_family = prometheus::BuildCounter().Name("profiler_samples").Help("async-profiler method samples").Register(*registry_)]
        (const std::string& caller_name, const std::string& callee_name) {
            return &metric_family.Add({{"caller", caller_name}, {"callee", callee_name}});
        }
    ) {}

    counter prometheus_store::get_or_create_counter(const std::string& caller, const std::string& callee) {
        if (!counters_.contains(caller) && !counters_[caller].contains(callee)) {
            counters_[caller][callee] = std::shared_ptr<prometheus::Counter>(counter_factory_(caller, callee));
        }
        return counter {counters_[caller][callee]};
    }

    
}


