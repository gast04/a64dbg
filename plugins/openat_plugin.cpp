#include <iostream>
#include <string.h>

#include "Plugins/Plugins.h"
#include "Plugins/PUtils.h"
#include "Utils/Utils.h"

class OpenatPlugin : public Plugin
{
public:

    void beforeSyscall() override {
        struct user_pt_regs regs = readRegisters();

        /*regs.regs[10] = 0x1337;
        writeRegisters(regs);
        struct user_pt_regs regs1 = readRegisters();
        dumpRegisters(regs1);
        */

        // get first syscall argument (careful with the bionic tag)
        uint64_t read_addr = regs.regs[1] & 0x00ffffffffffffff;
        uint8_t buffer[256] = {};
        readMemory((void*)read_addr, buffer, 256);
        // ignore return value
        printf("[P]   openat, arg: %s\n", buffer);

        if (strncmp((char*)buffer,
            "/data/app/~~2SqDfbmkxjgB0B9dlta15Q==/com.superplusgames.hos2-FgISVIxJbwG0abshmIBFoQ==/base.apk", 94) != 0) {
            return;
        }

        dumpRegisters(regs);

        // overwrite the argument
        const char* new_path = "/data/local/tmp/base_hos2.apk\x00";
        writeMemory((void*)read_addr, (uint8_t*)new_path, strlen(new_path)+1);

        // read addr again to verify that it got overwritten succesfully
        memset(buffer, 0, 256);
        readMemory((void*)read_addr, buffer, 256);
        // ignore return value
        printf("[P]   openat, modded arg: %s\n", buffer);
    }

    void afterSyscall() override {
        struct user_pt_regs regs = readRegisters();
        // dumpRegisters(regs);
        printf("[P]   openat, returned fd: %lu\n", regs.regs[0]);
    }

    static Plugin* create() {
        return new OpenatPlugin();
    }
};

REGISTER_PLUGIN(openat, OpenatPlugin::create);