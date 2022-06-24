#include "Utils/mem_checker.h"

OBSERVER_AREA* obs_list;
uint64_t obs_count;

FILE* proc_mem_read;
FILE* proc_mem_write;

// used for console output
int hits = 0;
uint64_t addr[MAX_HIT_PRINT];
uint64_t orig[MAX_HIT_PRINT];
uint64_t modd[MAX_HIT_PRINT];
char*    name[MAX_HIT_PRINT];

void getMemHandle(uint64_t pid, uint8_t readonly) {
  const char* fmt_mem = "/proc/%d/mem";
  char* str_mem = (char*)calloc(20, 1);
  sprintf(str_mem, fmt_mem, pid);

  proc_mem_read = fopen(str_mem, "rb");
  if (!proc_mem_read) {
    printf("ERROR: Could not obtain read handle for /proc/$(pid)/mem!\n");
    exit(-1);
  }

  if(readonly)
    return;

  proc_mem_write = fopen(str_mem, "wb");
  if (!proc_mem_write) {
    printf("ERROR: Could not obtain write handle for /proc/$(pid)/mem!\n");
    exit(-1);
  }
}

void seekToMem(uint64_t mem_start, FILE* mem_handle) {

  // SEEK_SET start from beginning of file
  int error = fseek(mem_handle, mem_start, SEEK_SET);
  if (error) {
    printf("could not seek to mem\n");
    exit(-2);
  }
}

int readMem(uint64_t* mem_storage, uint64_t mem_size) {
  uint64_t read = fread(mem_storage, 1, mem_size, proc_mem_read);
  // printf("mem Bytes Read: 0x%lx\n", read);
  if (read != mem_size) {
    printf("didnt read enough from Area! %lu != %lu\n", read, mem_size);
    //exit(-3);
    return 0;
  }
  return 1;
}

int readUserInput() {
  printf("\nWainting for Input...\n");
  printf("  'c' - changed\n");
  printf("  'u' - unchanged\n");
  printf("  'w' - write\n");
  printf("  'r' - restore\n");
  printf("  'o' - observe\n");
  printf("  'q' - quit\n");

  while(1) {
    char c = getchar();
    if (c == 'u' || c == 'U') {
      return UNCHANGED;
    }
    else if (c == 'c' || c == 'C') {
      return CHANGED;
    }
    else if (c == 'w' || c == 'W') {
      return WRITE_OP;
    }
    else if (c == 'r' || c == 'R') {
      return RESTORE_OP;
    }
    else if (c == 'o' || c == 'O') {
      return OBSERVE_OP;
    }
    else if (c == 'q' || c == 'Q') {
      printf("Quit Memory reader!\n");
      exit(0);
    }
    else if (c == '\n') {}
    else {
      printf("Unknown input: %c\n", c);
    }
  }
}

void initialMemoryread() {

  // do initial read of all areas which we observe
  for(uint64_t area_i = 0; area_i < obs_count; area_i++) {

    OBSERVER_AREA* area = &obs_list[area_i];
    seekToMem((uint64_t)area->addr_start, proc_mem_read);

    // alloc space for memory and check memory
    area->mem_ptr = (uint64_t*)malloc(area->length);
    area->cmp_ptr = (uint8_t*)calloc(area->length/8, 1);
    // -> divide by 8 cause 64bits are checked by a uint8_t

    // perform initial read
    if(!readMem(area->mem_ptr, area->length)) {
      // if we could not read, mark as invalid
      area->invalid = 1;
    }
    else {
      // mark as valid
      area->invalid = 0;
    }
  }
}

void invalidateMemory() {

  // check if not invalid areas have all invalid entries
  for(uint64_t area_i = 0; area_i < obs_count; area_i++) {

    OBSERVER_AREA* area = &obs_list[area_i];
    if(area->invalid)
      continue;

    int all_invalid = 1;
    for( uint64_t i = 0; i < (area->length/8); i ++) {
        if (area->cmp_ptr[i] == 0) {
          all_invalid = 0;
          break;
        }
    }

    if(!all_invalid) {
      printf("Area active: %p\t%s\n", area->addr_start, area->pathname);
      continue;
    }

    printf("Release Area: %p\t%s\n", area->addr_start, area->pathname);
    free(area->mem_ptr);
    free(area->cmp_ptr);
    area->invalid = 1;
  }
}

void checkForChanges(int mode) {

  hits = 0;
  for(uint64_t area_i = 0; area_i < obs_count; area_i++) {

      OBSERVER_AREA* area = &obs_list[area_i];
      if (area->invalid)
        continue;

      // read memory for comparission
      seekToMem((uint64_t)area->addr_start, proc_mem_read);
      uint64_t* tmp_mem = (uint64_t*)malloc(area->length);
      if(!readMem(tmp_mem, area->length)) {
        // mark as invalid if we could not read
        area->invalid = 1;
        free(tmp_mem);
        continue;
      }

      // iterate over memory
      for(uint64_t i = 0; i < (area->length/8); i++) {
        if (area->cmp_ptr[i] == 1)
          continue; // memory location already out

        if (mode == CHANGED) {
          if (area->mem_ptr[i] == tmp_mem[i]) {
            // values which stay the same are discarded
            area->cmp_ptr[i] = 1;
            continue;
          }
        }
        else if (mode == UNCHANGED) {
          if (area->mem_ptr[i] != tmp_mem[i]) {
            // values which changed are marked as out
            area->cmp_ptr[i] = 1;
            continue;
          }
        }

        // save values for printing
        if (hits < MAX_HIT_PRINT) {
          orig[hits] = area->mem_ptr[i];
          modd[hits] = tmp_mem[i];
          addr[hits] = (uint64_t)area->addr_start + i*8;
          name[hits] = (char*)area->pathname;
        }
        hits ++;

        // update memory for future iterations
        area->mem_ptr[i] = tmp_mem[i];
      } // iterate over memory loop

      // free temporar comparission memory
      free(tmp_mem);

    } // areas for-loop

    // after checking all memory, some might be invalid
    invalidateMemory();

    if(mode == CHANGED) {
      printf("Changed Memory Locations: %d\n", hits);
    }
    else {
      printf("UNchanged Memory Locations: %d\n", hits);
    }

    if (hits >= MAX_HIT_PRINT)
      return;

    // print only if we have less then MAX_HIT_PRINT hits
    for(int i = 0; i < hits; i ++) {
      printf("%016lx %016lx -> %016lx\t%s\n",
        addr[i], orig[i], modd[i], name[i]);
    }
}

void writeMemory() {
  uint64_t write_address;
  uint64_t write_value;

  printf("Pass value to write, Format <address> <value> (both in hex)\n");
  int read  = scanf("%lx %lx", &write_address, &write_value);
  printf("Write: %016lx -> %016lx\n", write_value, write_address);

  seekToMem(write_address, proc_mem_write);
  fwrite(&write_value, 8, 1, proc_mem_write);
}

void restoreMemory() {
  if (hits > MAX_HIT_PRINT) {
    printf("too many hits, not restore!\n");
    return;
  }

  // restore all hits from the last round
  for(int i = 0; i < hits; i ++) {

    uint64_t write_address = addr[i];
    uint64_t write_value   = orig[i];
    printf("Restore: %016lx - %016lx\n", write_address, write_value);

    seekToMem(write_address, proc_mem_write);
    fwrite(&write_value, 8, 1, proc_mem_write);
  }
}

void observeMemory() {
  if (hits > MAX_HIT_PRINT) {
    printf("too many hits, no observation!\n");
    return;
  }

  // observe all hits from the last round
  uint64_t read_value;
  for(int i = 0; i < hits; i ++) {

    uint64_t read_address = addr[i];

    seekToMem(read_address, proc_mem_read);
    uint64_t read = fread(&read_value, 1, 8, proc_mem_read);
    printf("Observe: %016lx - %016lx\n", read_address, read_value);
  }
}

void runMemChecking(
    OBSERVER_AREA* observer_list, uint64_t count, uint64_t pid) {

  obs_list = observer_list;
  obs_count = count;

  // obtain memory handle and perform initial read
  getMemHandle(pid, 0);
  initialMemoryread();

  // endless scanning/write/restore loop
  while(1) {
    int mode = readUserInput();

    if(mode == WRITE_OP) {
      writeMemory();
    }
    else if (mode == RESTORE_OP) {
      restoreMemory();
    }
    else if (mode == OBSERVE_OP) {
      observeMemory();
    }
    else {
      checkForChanges(mode);
    }
  }
}

void dumpMemory(uint64_t* mem_ptr, uint64_t address, uint64_t size, uint64_t pid) {
  getMemHandle(pid, 1);
  seekToMem(address, proc_mem_read);
  readMem(mem_ptr, size);
}
