#include <csignal>
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include <queue>
#include <unistd.h>
#include <sys/stat.h>

#include "prometheus/prometheus_store.hpp"
#include "core/reader.hpp"
#include "core/stack_trie.hpp"
#include "core/trace_parser.hpp"


using namespace yznal::trace_collector;

std::sig_atomic_t enabled = true;

// todo actual args
static int PROFILE_DURATION = 13;
static int MAX_LEVEL = 3;

static void errexit(const char* msg) {
    std::cerr << msg << '\n';
    exit(1);
}

struct arguments {
    std::string aync_profiler;
    int java_pid;
};



void sigint_handler(int signal) {
    enabled = false;
}

static int start_profiler(arguments& args) {
    char dur[3];
    sprintf(dur, "%d", PROFILE_DURATION);
    char pid[10];
    sprintf(pid, "%d", args.java_pid);
    //char* argv[] = {"-o", "collapsed", "-d", dur, pid, NULL};

    return execl(args.aync_profiler.c_str(), "_filler_", "-e", "wall", "-d", dur, "-i", "10ms", "-o", "collapsed", pid, NULL);
}

static void event_loop(arguments args) {
    prometheus_store prom;
    std::map<uint64_t, metric> counter_cache;

    std::shared_ptr<method_dict> dict(new method_dict());
    stack_trie trie(dict);

    std::string method = "org/yznal/profilingdemo/Common.doStuff";
    method_id target_id = dict->encode(method);
    node_id node_idx = -1;
    std::ofstream journal("journal.txt");
    auto refill = [&trie, &method, &journal](std::string&& str) {
        journal << str << '\n';
        stacktrace trace = parse_line(str);
        int idx = 0;
        for (const std::string& frame : trace.stack) {
            if (frame == method) {
                break;
            }
            idx++;
        }
        if (idx != trace.stack.size()) {
            trie.add_stacktrace(trace, idx);
        }
    };

    fifo_reader reader(STDIN_FILENO, std::function<void(std::string&&)>(refill));

    while (enabled) {
        int pfd[2];
        if (pipe(pfd) == -1 ) {
            errexit("could not create pipe");
        }

        pid_t child = fork();
        if (child == -1) {
            errexit("Fork failed");
        }

        if (child == 0) {
            close(pfd[0]);
            dup2(pfd[1], STDOUT_FILENO);

            int res = start_profiler(args);

            close(pfd[1]);
            exit(res);
        } else {
            close(pfd[1]);
            dup2(pfd[0], STDIN_FILENO);

            reader.process();

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

            auto& nodess = trie.get_nodes();
            
            // bfs
            if (node_idx != -1) {
                size_t target_total = trie.get_nodes()[node_idx].sample_count;
                int level = 1;
                std::queue<std::pair<node_id, int>> traverse_buffer;
                traverse_buffer.push( {node_idx, 0});
                std::string caller = dict->decode(target_id);

                while (!traverse_buffer.empty() && level <= MAX_LEVEL) {
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


            }
            
            //trie.print_debug();
            close(pfd[0]);
            trie.clear_invocations();
        }
        
        if (enabled) {
            // todo allign time, add cv
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        //break;
    } // while enabled

    std::cerr << "exit\n";
    std::this_thread::sleep_for(std::chrono::seconds(10));

}


int main(int argc, char** argv) {
    
    
    signal(SIGINT, sigint_handler);

    arguments args {argv[1], atoi(argv[2])};

    auto collector_thread = std::thread(event_loop, args);

    collector_thread.join();
    
    return 0;
}

