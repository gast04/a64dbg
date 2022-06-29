#include <iostream>

#include "Plugins/Plugins.h"

class OpenatPlugin : public Plugin
{
public:

    void beforeSyscall() override {
        std::cout << "before 'close' syscall\n";

        struct user_pt_regs regs = readRegisters();
        dumpRegisters(regs);

        regs.regs[10] = 0x1337;

        writeRegisters(regs);

        struct user_pt_regs regs1 = readRegisters();
        dumpRegisters(regs1);
    }

    void afterSyscall() override {
        std::cout << "after 'close' syscall\n";
    }

    static Plugin* create() {
        return new OpenatPlugin();
    }
private:

    void dumpRegisters(struct user_pt_regs regs) {
        printf("----Register Map-------------------------------------------\n");
        for (int i = 0; i < 10; ++i) {
            printf("  x%d:   0x%016llx   x%d:  0x%016llx  x%d:  0x%016llx \n",
                i, regs.regs[i], i+10, regs.regs[i+10], i+20, regs.regs[i+20]);
        }
        printf("  x%d:  0x%016llx\n", 30, regs.regs[30]);
        printf("-----------------------------------------------------------\n");
    }
};

REGISTER_PLUGIN(openat, OpenatPlugin::create);