#include <csignal>
#include <cstring>
#include <chrono>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>

#include "collector.hpp"


using namespace yznal::trace_collector;

std::sig_atomic_t enabled = true;

static int PROFILE_DURATION = 13;

static const char* PROMPT = 
    "Usage: collector [-i <interval>] -p <pid> -m <method> -b <profiler_binary>\n"
    "  -i               profiling intervals in milliseconds\n"
    "  -p | --pid       java process id\n"
    "  -m | --method    method to profile\n"
    "  -b | --binary    async-profiler binary path\n"
    "\n"
    "Example: collector -p 1234 -b async-profiler/build/bin/asprof -m org/yznal/profilingdemo/Common.doStuff -i 10\n";

static void errexit(const char* msg) {
    std::cerr << msg << '\n';
    exit(1);
}

struct arguments {
    int java_pid;
    std::string aync_profiler;
    std::string target_method;
    int period_ms;
};


void sigint_handler(int signal) {
    (void) signal;
    enabled = false;
}

static int start_profiler(arguments& args) {
    char dur[3];
    sprintf(dur, "%d", PROFILE_DURATION);
    char pid[10];
    sprintf(pid, "%d", args.java_pid);
    char period[13];
    int ms = args.period_ms < 0 ? 10 : args.period_ms;
    sprintf(period, "%dms", ms);
    //char* argv[] = {"-o", "collapsed", "-d", dur, pid, NULL};

    return execl(args.aync_profiler.c_str(), "async-profiler", "-e", "wall", "-d", dur, "-i", period, "-o", "collapsed", pid, NULL);
}

static void event_loop(arguments args) {

    collector collector(args.target_method);

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

            collector.process_profile_batch();

            close(pfd[0]);
        }
        
        if (enabled) {
            // todo allign time, add cv
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        //break;
    } // while enabled

    std::cerr << "exit\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

static arguments parse_arguments(int argc, char** argv) {
    int pid = -1;
    char* method = nullptr;
    char* binary = nullptr;
    int interval = -1;

    for (int arg = 1; arg < argc; arg += 2) {
        if (strcmp(argv[arg], "-p") == 0 || strcmp(argv[arg], "--pid") == 0) {
            if (arg+1 >= argc)
                errexit("No argument for pid");
            pid = std::stoi(argv[arg+1]);
        } else if (strcmp(argv[arg], "-m") == 0 || strcmp(argv[arg], "--method") == 0) {
            if (arg+1 >= argc)
                errexit("No argument for method");
            method = argv[arg+1];
        } else if (strcmp(argv[arg], "-b") == 0 || strcmp(argv[arg], "--binary") == 0) {
            if (arg+1 >= argc)
                errexit("No argument for binary");
            binary = argv[arg+1];
        } else if (strcmp(argv[arg], "-i") == 0) {
            if (arg+1 >= argc)
                errexit("No argument for interval");
            interval = std::stoi(argv[arg+1]);
        } else {
            std::cerr << "Unexpected argument " << argv[arg] << '\n';
            errexit(PROMPT);
        }
    }

    if (pid == -1) {
        std::cerr << "PID not provided";
        errexit(PROMPT);
    }
    if (method == nullptr) {
        std::cerr << "method not provided";
        errexit(PROMPT);
    }
    if (binary == nullptr) {
        std::cerr << "binary path not provided";
        errexit(PROMPT);
    } 
    

    return {
        pid, 
        binary,
        method,
        interval
        };

}


int main(int argc, char** argv) {
    
    
    signal(SIGINT, sigint_handler);

    arguments args = parse_arguments(argc, argv);

    auto collector_thread = std::thread(event_loop, args);

    collector_thread.join();
    
    return 0;
}

