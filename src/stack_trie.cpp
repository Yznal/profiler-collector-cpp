#include "stack_trie.hpp"

#include <queue>
#include <iostream>


namespace yznal::trace_collector {

    method_id method_dict::encode(const std::string& method_name) {
        auto node = dict_.insert(std::make_pair(method_name, 0));
        if (node.second) {
            int id = reverse_map_.size();
            reverse_map_.push_back(method_name);
            node.first->second = id;
            return id;
        } else {
            return node.first->second;
        }
    }

    const std::string& method_dict::decode(method_id id) const {
        return reverse_map_[id];
    }

}


namespace yznal::trace_collector {

    stack_trie::stack_trie() : dict_(new method_dict()) {
        method_id root = dict_->encode(ROOT);
        nodes_.emplace_back(root, 0);
    }

    stack_trie::stack_trie(std::shared_ptr<method_dict> ext_dict) : dict_(ext_dict) {
        method_id root = dict_->encode(ROOT);
        nodes_.emplace_back(root, 0);
    }

    void stack_trie::add_stacktrace(const sample_info& trace_sample, int offset) {
        node* curr_node = &nodes_[0];
        const std::vector<std::string> frames = trace_sample.st.stack;
        curr_node->sample_count += trace_sample.count;

        for (size_t f = offset; f < frames.size(); f++) {
            method_id m_id = dict_->encode(frames[f]);
            auto map_node = curr_node->children_.insert(std::make_pair(m_id, 0));

            if (map_node.second) {
                uint32_t new_node_id = create_node(m_id, trace_sample.count);
                map_node.first->second = new_node_id;
                curr_node = &nodes_[new_node_id];
            } else {
                uint32_t existing_node_id = map_node.first->second;
                nodes_[existing_node_id].sample_count += trace_sample.count;
                curr_node = &nodes_[existing_node_id];
            }
        }
    }

    const method_dict& stack_trie::get_dictionary() const {
        return *dict_;
    }

    void stack_trie::print_debug() const {

        std::queue<uint32_t> bfs;
        bfs.push(0);
        int level = 0;
        int level_size = 1;
        while (!bfs.empty()) {
            uint32_t node_id = bfs.front();
            bfs.pop();
            level_size--;

            const node& node = nodes_[node_id];
            std::cout << "[" << dict_->decode(node.current_method) << "," << node.sample_count << "]";
            for (const auto& [k, v] : node.children_) {
                bfs.push(v);
            }

            if (level_size == 0) {
                std::cout << '\n';
                level_size = bfs.size();
                level++;
                std::cout << "level: " << level << "\n";
            }
        }
        
    }

}
