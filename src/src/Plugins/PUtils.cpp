#include <iostream>

#include "Connector.h"
#include "Plugins/PUtils.h"

struct user_pt_regs readRegisters() {
    return Connector::getInstance().getRegisters();
}

void writeRegisters(struct user_pt_regs regs) {
    Connector::getInstance().setRegisters(regs);
}

size_t readMemory(void* addr, uint8_t* buffer, size_t size) {
    return Connector::getInstance().readMemory(addr, buffer, size);
}

size_t writeMemory(void* addr, uint8_t* buffer, size_t size) {
    return Connector::getInstance().writeMemory(addr, buffer, size);
}

// move to Utils/Utils.cpp
void dumpRegisters(struct user_pt_regs regs) {
    printf("----Register Map-------------------------------------------\n");
    for (int i = 0; i < 10; ++i) {
        printf("  x%d:   0x%016llx   x%d:  0x%016llx  x%d:  0x%016llx \n",
            i, regs.regs[i], i+10, regs.regs[i+10], i+20, regs.regs[i+20]);
    }
    printf("  x%d:  0x%016llx\n", 30, regs.regs[30]);
    printf("-----------------------------------------------------------\n");
}