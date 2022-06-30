#include "Utils/Memory.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// https://pubs.opengroup.org/onlinepubs/9699919799/functions/read.html

int getMemFd(uint64_t pid, bool read) {
    const char* fmt_mem = "/proc/%d/mem";
    char* str_mem = (char*)calloc(20, 1);
    sprintf(str_mem, fmt_mem, pid);

    int proc_mem_fd = -1;
    if (read) {
        proc_mem_fd = open(str_mem, O_RDONLY);  // READ ONLY
    }
    else {
        proc_mem_fd = open(str_mem, O_RDWR);    // READ/WRITE
    }

    //printf("[Memory] /proc/$(pid)/mem, fd: %d\n", proc_mem_fd);
    return proc_mem_fd;
}

ssize_t readProcMemory(pid_t pid, uint64_t offset, uint8_t* mem_storage,
                    uint64_t mem_size)
{
    int fd = getMemFd(pid, true);
    if (fd == -1) {
        return -1;
    }

    ssize_t read = pread(fd, mem_storage, mem_size, offset);
    if (read != mem_size) {
        printf("[Memory] not enough read! %lu != %lu\n", read, mem_size);
    }
    close(fd);
    return read;
}

ssize_t writeProcMemory(pid_t pid, uint64_t offset, uint8_t* mem_storage,
                        uint64_t mem_size)
{
    int fd = getMemFd(pid, false);
    if (fd == -1) {
        return -1;
    }

    // TODO: debug mode
    printf("write to address: %lx, %lu\n", offset, mem_size);
    printf("%s\n", (char*)mem_storage);

    ssize_t written = pwrite(fd, mem_storage, mem_size, offset);
    if (written == -1 || written != mem_size) {
        printf("[Memory] not enough written! %lu != %lu\n", written, mem_size);
    }

    close(fd);
    return written;
}