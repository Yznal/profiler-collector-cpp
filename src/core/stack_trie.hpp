#pragma once

#include "trace_parser.hpp"

#include <vector>
#include <string>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <memory>


namespace yznal::trace_collector {

    using method_id = int32_t;
    using node_id = int32_t;

    class method_dict {
    public:
        method_id encode(const std::string& method_name);

        const std::string& decode(method_id id) const;

    private:
        std::unordered_map<std::string, method_id> dict_;
        std::vector<std::string> reverse_map_;
    };


    class stack_trie {
    public:
        static inline const std::string ROOT = "[stack_trace_root]";

        struct node {

            node(method_id m_id, node_id parent = 0, uint64_t cnt = 0) : parent(parent), current_method(m_id), sample_count(cnt) {
            }

            std::map<method_id, node_id> children_;
            node_id parent;
            method_id current_method; 
            uint64_t sample_count;
        };

        stack_trie();
        explicit stack_trie(std::shared_ptr<method_dict>);

        void add_stacktrace(const stacktrace& trace_sample, int offset = 0);
        method_dict& get_dictionary();
        const method_dict& get_dictionary() const;
        //const std::vector<node>& get_nodes();
        const std::vector<node>& get_nodes() const;

        void clear_invocations();

        void print_debug() const;

    private:
        std::shared_ptr<method_dict> dict_;
        std::vector<node> nodes_;

        uint32_t create_node(method_id m_id,  node_id parent, uint64_t cnt);

    };


}