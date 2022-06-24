#include "Utils/Memory.h"

FILE* getMemReadHandle(uint64_t pid) {
  const char* fmt_mem = "/proc/%d/mem";
  char* str_mem = (char*)calloc(20, 1);
  sprintf(str_mem, fmt_mem, pid);

  FILE* proc_mem_read = fopen(str_mem, "rb");
  if (!proc_mem_read) {
    printf("ERROR: Could not obtain read handle for /proc/$(pid)/mem!\n");
    exit(-1);
  }

  return proc_mem_read;
}

void seekToMem(uint64_t mem_start, FILE* mem_handle) {

  // SEEK_SET start from beginning of file
  int error = fseek(mem_handle, mem_start, SEEK_SET);
  if (error) {
    printf("could not seek to mem\n");
    exit(-2);
  }
}

bool readMemory(pid_t pid, uint64_t offset, uint32_t* mem_storage, uint64_t mem_size) {

    FILE* fmem = getMemReadHandle(pid);
    seekToMem(offset, fmem);

    uint64_t read = fread(mem_storage, 1, mem_size, fmem);
    if (read != mem_size) {
        printf("[Memory] didnt read enough! %lu != %lu\n", read, mem_size);
        return false;
    }
    return true;
}