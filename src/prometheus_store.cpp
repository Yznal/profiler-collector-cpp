#include "prometheus_store.hpp"



namespace yznal::trace_collector {



    prometheus_store::prometheus_store() {
        prometheus::BuildCounter().Name("Awoga").Register(*registry_).Add({{"biba", "boba"}}).Increment()
    }

    void prometheus_store::record_sample(const std::string& caller, const std::string& callee, size_t samples_count);

    
}


