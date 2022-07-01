#include <stdio.h>
#include <sys/mman.h>

#include "Connector.h"
#include "Utils/Memory.h"


void Connector::init(pid_t tracee_pid) {
    tracee = tracee_pid;
    private_memory = -1;
}

void Connector::setTracee(pid_t tracee_pid) {
    tracee = tracee_pid;
}

bool Connector::attach() {
    if (ptrace(PTRACE_ATTACH, tracee, 0, 0) < 0) {
        printf("[!] Error in PTRACE_ATTACH\n");
        return false;
    }
    return true;
}

struct user_pt_regs Connector::getRegisters() {
    struct user_pt_regs regs;
    struct iovec io;
    io.iov_base = &regs;
    io.iov_len = sizeof(regs);

    // NT_PRSTATUS general purpose registers
    if (ptrace(PTRACE_GETREGSET, tracee, (void*)NT_PRSTATUS, (void*)&io) < 0) {
        printf("[!] Error in PTRACE_GETREGSET\n");
        // TODO: return proper error value
        return regs;
    }

    return regs;
}

void Connector::setRegisters(struct user_pt_regs regs) {
    struct iovec io;
    io.iov_base = &regs;
    io.iov_len = sizeof(regs);

    // NT_PRSTATUS general purpose registers
    if (ptrace(PTRACE_SETREGSET, tracee, (void*)NT_PRSTATUS, (void*)&io) < 0) {
        printf("[!] Error in PTRACE_SETREGSET\n");
    }
}

size_t Connector::readMemory(void* addr, uint8_t* buffer, size_t size) {
    // TODO: check if it possible to read from this addr
    return readProcMemory(tracee, (uint64_t)addr, buffer, size);
}

size_t Connector::writeMemory(void* addr, uint8_t* buffer, size_t size) {
    // TODO: check if it possible to write to this addr
    return writeProcMemory(tracee, (uint64_t)addr, buffer, size);
}

uint32_t Connector::getDWORD(void* addr) {
    return ptrace(PTRACE_PEEKTEXT, tracee, addr, 0);
}

uint64_t Connector::getTraceePid() {
    return tracee;
}

uint64_t Connector::getPrivateMemory() {
    return private_memory;
}

int Connector::doSingleStep()
{
    int wait_status = 0;
    if (ptrace(PTRACE_SINGLESTEP, tracee, 0, 0) < 0) {
        printf("[!] Error in PTRACE_SINGLESTEP\n");
        return -1;
    }

    waitpid(tracee, &wait_status, 0);
    return wait_status;
}

uint64_t Connector::allocateMemoryInChild()
{
    struct user_pt_regs regs = getRegisters();
    struct user_pt_regs regs_save = regs; // for later restore

    // write the 'svc #0' instruction, 01 00 00 D4
    uint64_t orig_inst = ptrace(PTRACE_PEEKTEXT, tracee, (void*)regs.pc, 0);
    uint64_t aarch464_svc = 0xD4000001;
    ptrace(PTRACE_POKETEXT, tracee, (void*)regs.pc, (void*)aarch464_svc);

    // setup registers for mmap syscall
    regs.regs[8] = 222;     // syscall number
    regs.regs[0] = 0;       // address to modify
    regs.regs[1] = 1024;    // size
    regs.regs[2] = 7;       // protection flags (rwx) (5 rx, 3 rw, 1 r)
    regs.regs[3] = MAP_ANONYMOUS | MAP_PRIVATE;
    regs.regs[4] = -1;      // fd
    regs.regs[5] = 0;       // offset
    setRegisters(regs);

    // do single step and execute mmap
    if (doSingleStep() == -1) {
        return -1;
    }

    // read registers to get mmap address
    regs = getRegisters();
    private_memory = regs.regs[0];
    printf("[+] Privat memory: 0x%lx\n", private_memory);

    // restore registers and instruction
    setRegisters(regs_save);
    ptrace(PTRACE_POKETEXT, tracee, (void*)regs_save.pc, (void*)orig_inst);

    // directly write `svc #0` (01 00 00 D4) instruction to mmaped area, as its
    // mainly used for sycalls
    ptrace(PTRACE_POKETEXT, tracee, (void*)private_memory, (void*)aarch464_svc);

    return private_memory;
}

uint64_t Connector::mprotectMemory(uint64_t mem_addr, uint64_t mem_size,
                               uint64_t mem_prot)
{
    if (private_memory == -1) {
        printf("[!] Private Memory not set, cannot call mprotect!\n");
        return -1;
    }

    if (mem_addr == private_memory) {
        printf("[!] Mprotect on private memory not possible!\n");
        return -1;
    }

    // NOTE: private memory already contains the svc instruction
    struct user_pt_regs regs = getRegisters();
    struct user_pt_regs regs_save = regs; // for restore

    // setup registers for mprotect syscall
    regs.pc = private_memory;
    regs.regs[8] = 226;      // syscall number
    regs.regs[0] = mem_addr; // address to modify
    regs.regs[1] = mem_size; // size
    regs.regs[2] = mem_prot; // 7 rwx, 5 rx, 3 rw, 1 r, 0 NONE
    setRegisters(regs);

    // do single step and execute mprotect
    if (doSingleStep() == -1) {
        return -1;
    }

    // read registers to get mprotect result
    regs = getRegisters();
    uint64_t res = regs.regs[0];

    // restore registers
    setRegisters(regs_save);
    return res;
}
