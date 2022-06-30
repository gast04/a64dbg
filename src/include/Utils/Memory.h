#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

ssize_t readProcMemory(pid_t pid, uint64_t offset, uint8_t* mem_storage, uint64_t mem_size);
ssize_t writeProcMemory(pid_t pid, uint64_t offset, uint8_t* mem_storage, uint64_t mem_size);
