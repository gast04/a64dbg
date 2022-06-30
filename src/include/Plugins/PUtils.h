#include <sys/ptrace.h>


void dumpRegisters(struct user_pt_regs regs);
struct user_pt_regs readRegisters();
void writeRegisters(struct user_pt_regs regs);
size_t readMemory(void* addr, uint8_t* buffer, size_t size);
size_t writeMemory(void* addr, uint8_t* buffer, size_t size);

