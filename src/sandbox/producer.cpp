#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <iostream>
#include "globals.h"

int main(int argc, char** argv) {
    (void) argc;
    (void) argv;


    int fd = open(globals::fifo_name, O_WRONLY);
    if (fd == -1) {
        if (errno == ENOENT)
            std::cerr << "fifo does not exist\n";
        else 
            std::cerr << errno <<  " meh\n";
        exit(-1);
    }
    char* buf = new char[100];
    for (int i = 0; i < 10; i++) {
        int msg_size = snprintf(buf, 100, "Message %d: %d sheep escaped\n", i, i);
        ssize_t w = write(fd, buf, msg_size);
        (void) w;
    }

    delete[] buf;

}

