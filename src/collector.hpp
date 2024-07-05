#pragma once

#include <cstdint>

#include "prometheus/prometheus_store.hpp"
#include "core/reader.hpp"
#include "core/stack_trie.hpp"
#include "core/trace_parser.hpp"


namespace yznal::trace_collector {

    class collector {
    public:

        collector(std::string target_method, int max_level = 3);

        void process_profile_batch();


    private:

        void refill_metrics(const std::string& str);

        void render_metrics();

    private:
        prometheus_store prom;
        std::map<uint64_t, metric> counter_cache;
        std::shared_ptr<method_dict> dict;
        stack_trie trie;
        std::string method;
        node_id node_idx = -1;
        method_id target_id = -1;
        int max_render_level;
        fifo_reader reader;



    };
    
}