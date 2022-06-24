#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

bool readMemory(pid_t pid, uint64_t offset, uint32_t* mem_storage, uint64_t mem_size);
