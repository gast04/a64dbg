#include <stdio.h>

#include "Connector.h"

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

    ptrace(PTRACE_GETREGSET, tracee, (void*)NT_PRSTATUS, (void*)&io);

    return regs;
}
