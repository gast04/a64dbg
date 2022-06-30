#include <iostream>

#include "Plugins/Plugins.h"
#include "Plugins/PUtils.h"
#include "Utils/Utils.h"

class FaccessatPlugin : public Plugin
{
public:

    void beforeSyscall() override {
        //std::cout << "[P] before 'faccessat' syscall\n";

        struct user_pt_regs regs = readRegisters();
        uint64_t read_addr = regs.regs[1] & 0x00ffffffffffffff;
        uint8_t buffer[256] = {};
        readMemory((void*)read_addr, buffer, 256);
        printf("[P]   faccessat, arg: %s\n", buffer);
    }

    void afterSyscall() override {
        //std::cout << "after 'faccessat' syscall\n";
    }

    static Plugin* create() {
        return new FaccessatPlugin();
    }
};

REGISTER_PLUGIN(faccessat, FaccessatPlugin::create);