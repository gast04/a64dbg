#ifndef H_MEM_CHECKER
#define H_MEM_CHECKER

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#define UNCHANGED  1
#define CHANGED    2
#define WRITE_OP   3
#define RESTORE_OP 4
#define OBSERVE_OP 5
#define MAX_HIT_PRINT 300

struct observer_area {
  void* addr_start;           // start address of the area
  void* addr_end;             // end address
  unsigned long length;       // size of the range
  // permission is at least rw?
  char pathname[200];         // keep pathname for better printing

  uint8_t invalid; // marks area as invalid and skips scanning
  uint64_t* mem_ptr;
  uint8_t* cmp_ptr;
};
typedef struct observer_area OBSERVER_AREA;
typedef OBSERVER_AREA* OBSERVER_LIST;

void dumpMemory(uint64_t* mem_ptr, uint64_t address, uint64_t size, uint64_t pid);

void runMemChecking(OBSERVER_AREA* observer_area, uint64_t count, uint64_t pid);

#endif // H_MEM_CHECKER
