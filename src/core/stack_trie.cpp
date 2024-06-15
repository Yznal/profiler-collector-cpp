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
        create_node(root, 0, 0);
    }

    stack_trie::stack_trie(std::shared_ptr<method_dict> ext_dict) : dict_(ext_dict) {
        method_id root = dict_->encode(ROOT);
        create_node(root, 0, 0);
    }

    uint32_t stack_trie::create_node(method_id m_id, node_id parent, uint64_t cnt) {
        size_t node_id = nodes_.size();
        nodes_.emplace_back(m_id, parent, cnt);
        return static_cast<uint32_t>(node_id);
    }

    void stack_trie::add_stacktrace(const stacktrace& trace_sample, int offset) {
        node_id cur_node_id = 0;
        const std::vector<std::string> frames = trace_sample.stack;
        nodes_[cur_node_id].sample_count += trace_sample.count;

        for (size_t f = offset; f < frames.size(); f++) {
            method_id m_id = dict_->encode(frames[f]);
            auto map_node = nodes_[cur_node_id].children_.insert(std::make_pair(m_id, 0));

            if (map_node.second) {
                node_id new_node_id = create_node(m_id, cur_node_id, trace_sample.count);
                map_node.first->second = new_node_id;
                cur_node_id = new_node_id;
            } else {
                node_id existing_node_id = map_node.first->second;
                nodes_[existing_node_id].sample_count += trace_sample.count;
                cur_node_id =existing_node_id;
            }
        }
    }

    method_dict& stack_trie::get_dictionary() {
        return *dict_;
    }

    const method_dict& stack_trie::get_dictionary() const {
        return *dict_;
    }

    const std::vector<stack_trie::node>& stack_trie::get_nodes() const {
        return nodes_;
    }

    void stack_trie::clear_invocations() {
        for (auto& node : nodes_)
            node.sample_count = 0;
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
            std::cout << "[" << dict_->decode(node.current_method) << "," << node.sample_count << "/" << nodes_[node.parent].sample_count << "]";
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
