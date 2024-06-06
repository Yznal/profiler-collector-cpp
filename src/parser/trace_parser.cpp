#include "trace_parser.hpp"

#include <cstring>
#include <string>

namespace yznal::trace_collector {

    sample_info parse_line(const char* line) {
        const char* sep = strchr(line, ' ');
        sample_info info;
        
        size_t num = atoll(sep + 1);
        info.count = num;


        ptrdiff_t start_offset = 0, offset = 0;

        while (line + offset != sep) {
            if (line[offset] == ';') {
                info.st.stack.emplace_back(line + start_offset, offset - start_offset);
                offset++;
                start_offset = offset;
            } else
                offset++;
        }
        if (offset > start_offset) {
            info.st.stack.emplace_back(line + start_offset, offset - start_offset);
        }

        return info;
    }
}