#include <iostream>

#include "Plugins/Plugins.h"
#include "Plugins/PUtils.h"
#include "Utils/Utils.h"

class FstatatPlugin : public Plugin
{
public:

    void beforeSyscall() override {

        struct user_pt_regs regs = readRegisters();

        // get first syscall argument (careful with the bionic tag)
        uint64_t read_addr = regs.regs[1] & 0x00ffffffffffffff;
        uint8_t buffer[256] = {};
        readMemory((void*)read_addr, buffer, 256);
        // ignore return value
        printf("[P]   fstatat, arg: %s\n", buffer);
    }

    void afterSyscall() override {
    }

    static Plugin* create() {
        return new FstatatPlugin();
    }
};

REGISTER_PLUGIN(fstatat, FstatatPlugin::create);