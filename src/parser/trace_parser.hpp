#pragma once

#include <string>
#include <vector>
#include <cstddef>


namespace yznal::trace_collector {

    struct stacktrace {
        std::vector<std::string> stack;
    };

    struct sample_info {
        stacktrace st;
        size_t count;
    };

    sample_info parse_line(const char* line);

}