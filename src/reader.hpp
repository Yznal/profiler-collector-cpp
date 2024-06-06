#pragma once

#include <vector>
#include <cstddef>
#include <string>

namespace yznal::trace_collector {

    using consumer = void(std::string&& str);

    class fifo_reader {
    public:
        
        // todo: separator?
        fifo_reader(const char* fifo_name, consumer* consumer_func, size_t buffer_size = 4096) noexcept;

        fifo_reader(int fd, consumer* consumer_func, size_t buffer_size = 4096) noexcept;

        // no cpy, single resource owner
        fifo_reader(const fifo_reader&) = delete;
        fifo_reader& operator=(const fifo_reader&) = delete;

        fifo_reader(fifo_reader&&) = default;
        fifo_reader& operator=(fifo_reader&&) = default;

        ~fifo_reader();

        void process();

        bool is_open() const;

    private:

        void close();
        void open();

        consumer* consumer_func_;
        int read_fd_ = -1;
        const char* filename_;
        std::vector<char> buffer_;


    };

}