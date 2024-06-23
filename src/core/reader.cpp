#include "reader.hpp"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <iostream>

namespace yznal::trace_collector {

    fifo_reader::fifo_reader(int fd, std::function<void(const std::string&)> consumer_func, size_t buffer_size) noexcept : consumer_func_(std::move(consumer_func)), read_fd_(fd), buffer_(buffer_size) {
    }

    void fifo_reader::process() {
        size_t start = 0, end = 0;
        ssize_t rd;
        char* buf = buffer_.data();
        size_t b_size = buffer_.capacity();

        // std::string line;
        
        // read(read_fd_, buf, 0);
        // while (std::getline(std::cin, line)) {
        //     consumer_func_(line);
        // }

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
                    buf = buffer_.data();
                } else {
                    size_t sz = end - start;
                    memmove(buf, &buf[start], sz);
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