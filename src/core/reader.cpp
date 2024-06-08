#include "reader.hpp"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>

namespace yznal::trace_collector {

    fifo_reader::fifo_reader(int fd, consumer* consumer_func, size_t buffer_size) noexcept : consumer_func_(consumer_func), read_fd_(fd), buffer_(buffer_size) {
    }

    void fifo_reader::process() {
        size_t start = 0, end = 0;
        ssize_t rd;
        char* buf = buffer_.data();
        size_t b_size = buffer_.capacity();

        while ((rd = read(read_fd_, buf + end, b_size - end)) > 0) {
            size_t border = end + rd < b_size ? end + rd : b_size;
            while (end < border) {
                while (end < border && buf[end] != '\n') {
                    end++;
                }

                if (end < b_size && buf[end] == '\n') {
                    std::string st(&buf[start], end - start);
                    consumer_func_(std::move(st)); 

                    end++;
                    start = end;
                }

                if (end == b_size)
                    break;
                
            }
            // flip bufer
            if (start != end) {
                if (start == 0) {
                    // buffer full
                    // resize
                    buffer_.reserve(buffer_.capacity() * 2);
                } else {
                    size_t sz = end - start;
                    strncpy(buf, &buf[start], sz);
                    start = 0;
                    end = sz;
                }
            } else {
                start = 0;
                end = 0;
            }
        }
        // leftovers
        if (end > start) {
            std::string st(&buf[start], end - start);
            consumer_func_(std::move(st)); 
        }
    }

    bool fifo_reader::is_open() const {
        return read_fd_ >= 0;
    }

}