#pragma once

#include <string>
#include <vector>
#include <cstddef>


namespace yznal::trace_collector {

    struct stacktrace {
        std::vector<std::string> stack;
        int64_t t_id = -1;
        std::string t_name;
    };

    struct sample_info {
        stacktrace st;
        size_t count;
    };

    sample_info parse_line(const std::string& line, bool threaded = false);

}