#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <iostream>

#include "globals.h"


static void unlinkFifo() {
    int ul = unlink(globals::fifo_name);
    (void) ul;
}

int main(int argc, char** argv) {
    (void) argc;
    (void) argv;

    if (mkfifo(globals::fifo_name, 0666) < 0) {
        std::cout << "Failed to open fifo " << errno << '\n';
        exit(-1);
    }
    if (atexit(unlinkFifo) != 0) {
        std::cerr << "Failed to append exit handler\n";
        exit(-1);
    }

    std::cerr << "Try opening fifo\n";
    int fd = open(globals::fifo_name, O_RDONLY);
    if (fd == -1) {
        std::cerr << errno << " meh\n";
        exit(-1);
    }
    std::cerr << "Opend fifo\n";

    char buf[1024];
    size_t start = 0, end = 0;
    ssize_t rd;

    while ((rd = read(fd, buf, sizeof(buf))) > 0) {
        while ((ssize_t)end < rd) {
            while (buf[end] != '\n') {
                end++;
            }
            std::string st;
            st.append(&buf[start], end - start);
            std::cout << "Received message: " << st << "\n";

            end++;
            start = end;
        }
        // some left
        if (start != end) {
            if (start == 0) {
                std::cerr << "Buffer overflow\n";
                exit(-1);
            }
            size_t sz = end - start;
            strncpy(buf, &buf[start], sz);
            start = 0;
            end = sz;
        } else {
            start = 0;
            end = 0;
        }
    }
    std::cout << "finished!\n";
    unlink(globals::fifo_name);

}
