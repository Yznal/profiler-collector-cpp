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


#include <csignal>



using namespace yznal::trace_collector;

void print_stack(std::string&& line) {
    sample_info info = parse_line(line.data());
    for (auto& frame : info.st.stack) {
        std::cout << frame << '\n';
    }
    std::cout << "Stack occurance: " << info.count << "\n\n";
}

stack_trie trie;

void save_stack(std::string&& line) {
    sample_info info = parse_line(line.data());
    trie.add_stacktrace(info);
}

int main(int argc, char** argv) {
    (void) argc;
    (void) argv;

    fifo_reader reader(STDIN_FILENO, save_stack, 4096);
    reader.process();

    trie.print_debug();

    return 0;
}
