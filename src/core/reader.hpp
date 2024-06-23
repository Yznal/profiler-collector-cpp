#pragma once

#include <vector>
#include <cstddef>
#include <string>
#include <functional>

namespace yznal::trace_collector {

class fifo_reader {
    public:
        

        fifo_reader(int fd, std::function<void(const std::string&)> consumer_func, size_t buffer_size = 4096) noexcept;

        // no cpy, single resource owner
        fifo_reader(const fifo_reader&) = delete;
        fifo_reader& operator=(const fifo_reader&) = delete;

        fifo_reader(fifo_reader&&) = default;
        fifo_reader& operator=(fifo_reader&&) = default;

        void process();

        bool is_open() const;

    private:

        void close();
        void open();

        std::function<void(const std::string&)> consumer_func_;
        int read_fd_ = -1;
        std::vector<char> buffer_;


    };

}
