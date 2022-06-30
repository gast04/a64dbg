#include <stdio.h>

#include "Connector.h"
#include "Utils/Memory.h"

void Connector::init(pid_t tracee_pid) {
    tracee = tracee_pid;
}

bool Connector::attach() {
    printf("[!] Connector: attach NOT implemented!\n");
    return false;
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
