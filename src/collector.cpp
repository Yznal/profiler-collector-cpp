#include "collector.hpp"

#include <queue>
#include <iostream>


namespace yznal::trace_collector {

    collector::collector(std::string target_method, int max_level) 
    : dict(new method_dict()), 
    trie(dict), 
    method(target_method),
    max_render_level(max_level),
    reader(STDIN_FILENO, [this] (const std::string& str) { this->refill_metrics(str); })
    {
        target_id = dict->encode(method);
    }

    void collector::refill_metrics(const std::string& str) {
        stacktrace trace = parse_line(str);
        int idx = 0;
        for (const std::string& frame : trace.stack) {
            if (frame == method) {
                break;
            }
            idx++;
        }
        if (idx != static_cast<int>(trace.stack.size())) {
            trie.add_stacktrace(trace, idx);
        }
    }

    void collector::render_metrics() {
        method_id target_id = trie.get_nodes()[node_idx].current_method;
        size_t target_total = trie.get_nodes()[node_idx].sample_count;
        int level = 1;
        std::queue<std::pair<node_id, int>> traverse_buffer;
        traverse_buffer.push( {node_idx, 0});
        std::string caller = dict->decode(target_id);

        while (!traverse_buffer.empty() && level <= max_render_level) {
            size_t level_total = target_total;
            size_t level_count = traverse_buffer.size();

            for (size_t i = 0; i < level_count; i++) {
                node_id parent_id = traverse_buffer.front().first;
                int synthetic_count = traverse_buffer.front().second;
                traverse_buffer.pop();
                auto& parent_node = trie.get_nodes()[parent_id];
                size_t parent_total = synthetic_count > 0 ? synthetic_count : parent_node.sample_count;
                if (synthetic_count == 0) { // non-synthetic
                    for (const auto& [method_id, node_id] : parent_node.children_) {
                        
                        auto& submethod_node = trie.get_nodes()[node_id];
                        uint64_t counter_key = (static_cast<uint64_t>(level) << 56) | (static_cast<uint64_t>(target_id) << 31) | (submethod_node.current_method);
                        if (!counter_cache.contains(counter_key)) {
                            std::string callee = dict->decode(submethod_node.current_method);
                            counter_cache.insert(std::make_pair(counter_key, prom.create_counter(caller, callee, level)));
                        }
                        
                        // todo filter zero?
                        counter_cache[counter_key].reset();
                        counter_cache[counter_key].increment(submethod_node.sample_count);
                        parent_total -= submethod_node.sample_count;
                        level_total -= submethod_node.sample_count;


                        traverse_buffer.push( {node_id, 0} );
                    }
                }

                if (parent_total > 0) { // synthetic
                    traverse_buffer.push({ parent_id, parent_total});
                    uint64_t counter_key = (static_cast<uint64_t>(level) << 56) | (static_cast<uint64_t>(target_id) << 31) | (parent_node.current_method);
                    if (!counter_cache.contains(counter_key)) {
                        const std::string& callee = dict->decode(parent_node.current_method);
                        counter_cache.insert(std::make_pair(counter_key, prom.create_counter(caller, callee, level)));
                    }
                    counter_cache[counter_key].reset();
                    counter_cache[counter_key].increment(parent_total);

                    level_total -= parent_total;
                }
            }

            level++;
        }
    };

    void collector::process_profile_batch() {
        try {
            reader.process();
        } catch(...) {
            std::cerr << "[Error] Damaged batch. Skip processing\n";
            reader.skip_input();
            for (auto& [k, v] : counter_cache) {
                v.reset();
            }
            trie.clear_invocations();
            return;
        }

        if (node_idx == -1) {
            int idx = 0;
            for (auto& node : trie.get_nodes()) {
                if (node.current_method == target_id) {
                    node_idx = idx;
                    break;
                }
                idx++;
            }
        }
        
        // bfs
        if (node_idx != -1) {
            render_metrics();

        }
        
        //trie.print_debug();
        trie.clear_invocations();
    }



}

