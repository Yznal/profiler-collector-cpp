#include "trace_parser.hpp"

#include <string>

namespace yznal::trace_collector {

    const char frame_separator = ';';

    sample_info parse_line(const std::string& line, bool threaded) {
        
        sample_info info;

        size_t sep_pos = line.find_last_of(' ');
        size_t num = std::stoll(line.data() + sep_pos + 1);
        info.count = num;    

        ptrdiff_t start_offset = 0, offset = 0;

        if (threaded) {
            // parse thread frame
            while (line[offset] != frame_separator) {
                offset++;
            }
            // [smth smth smth tid=%ld]
            size_t tid_sep_num = line.find_last_of(' ', offset);
            info.st.t_id = std::stoll(line.data() + tid_sep_num + 5);
            info.st.t_name = line.substr(1, tid_sep_num - 1);
            offset++;
            start_offset = offset;

        }
    

        while (offset != sep_pos) {
            if (line[offset] == frame_separator) {
                info.st.stack.emplace_back(line.substr(start_offset, offset - start_offset));
                offset++;
                start_offset = offset;
            } else
                offset++;
        }
        if (offset > start_offset) {
            info.st.stack.emplace_back(line.substr(start_offset, offset - start_offset));
        }

        return info;
    }
}