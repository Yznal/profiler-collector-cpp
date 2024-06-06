#pragma once

#include "parser/trace_parser.hpp"

#include <vector>
#include <string>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <memory>


namespace yznal::trace_collector {

    using method_id = int32_t;
    using node_id = uint32_t;

    class method_dict {
    public:
        method_id encode(const std::string& method_name);

        const std::string& decode(method_id id) const;

    private:
        std::unordered_map<std::string, method_id> dict_;
        std::vector<std::string> reverse_map_;
    };


    class stack_trie {
    private:

        static inline const std::string ROOT = "[stack_trace_root]";

        struct node {

            node(method_id m_id, uint64_t cnt = 0) : current_method(m_id), sample_count(cnt) {
            }

            std::map<method_id, node_id> children_;
            method_id current_method; 
            uint64_t sample_count;
        };


    public:

        stack_trie();
        explicit stack_trie(std::shared_ptr<method_dict>);

        void add_stacktrace(const sample_info& trace_sample, int offset = 0);
        const method_dict& get_dictionary() const;
        void print_debug() const;

    private:
        std::shared_ptr<method_dict> dict_;
        std::vector<node> nodes_;

        uint32_t create_node(method_id m_id, uint64_t cnt) {
            size_t node_id = nodes_.size();
            nodes_.emplace_back(m_id, cnt);
            return static_cast<uint32_t>(node_id);
        }

    };


}