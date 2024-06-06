#include <iostream>

#include "reader.hpp"
#include "parser/trace_parser.hpp"
#include "stack_trie.hpp"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <map>
#include <memory>


#include <csignal>



using namespace yznal::trace_collector;

void print_stack(std::string&& line) {
    stacktrace info = parse_line(line);
    for (auto& frame : info.stack) {
        std::cout << frame << '\n';
    }
    std::cout << "Stack occurance: " << info.count << "\n\n";
}

std::shared_ptr<method_dict> dict = std::shared_ptr<method_dict>(new method_dict());
stack_trie trie(dict);
std::map<int64_t, stack_trie> per_thread_trie;
std::map<int64_t, std::string> t_names;

void save_stack(std::string&& line) {
    stacktrace info = parse_line(line, true);
    trie.add_stacktrace(info);
    if (!per_thread_trie.contains(info.t_id)) {
        auto res = per_thread_trie.insert(std::make_pair(info.t_id, stack_trie(dict)));
        res.first->second.add_stacktrace(info);
        t_names[info.t_id] = info.t_name;
    } else {
        per_thread_trie[info.t_id].add_stacktrace(info);
    }
}

int main(int argc, char** argv) {
    (void) argc;
    (void) argv;

    fifo_reader reader(STDIN_FILENO, save_stack, 4096);
    reader.process();

    trie.print_debug();

    for (const auto& [t_id, tr] : per_thread_trie) {
        std::cout << '\n' << t_names[t_id] << " tid=" << t_id << "\n";
        tr.print_debug();
    }

    return 0;
}
