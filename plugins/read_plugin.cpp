#include <iostream>

#include "Plugins/Plugins.h"
#include "Plugins/PUtils.h"
#include "Utils/Utils.h"

class ReadPlugin : public Plugin
{
public:

    void beforeSyscall() override {
        struct user_pt_regs regs = readRegisters();
        dumpRegisters(regs);

        buffer_ptr = regs.regs[1] & 0x00ffffffffffffff;
        size = regs.regs[2];
        printf("[P]   read, fd: %lu, buff: %lx, size: %d\n",
              regs.regs[0], buffer_ptr, size);
    }

    void afterSyscall() override {
        std::cout << "after 'read' syscall\n";

        uint8_t buffer[256] = {};
        if (size > 256)
            size = 256;

        readMemory((void*)buffer_ptr, buffer, size);
        hexdump(buffer, buffer_ptr, size, 4);
    }

    static Plugin* create() {
        return new ReadPlugin();
    }

private:
    uint64_t buffer_ptr;
    uint64_t size;
};

REGISTER_PLUGIN(read, ReadPlugin::create);