/*
    goal is to register plugins which modify the input registers of a syscall
    so we want to be able to intercept syscalls

    https://dxuuu.xyz/cpp-static-registration.html#function-1
*/

#include <iostream>
#include "Plugins/Plugins.h"

class OpenPlugin : public Plugin
{
public:

    void beforeSyscall() {
        std::cout << "before 'open' syscall\n";

        struct user_pt_regs regs = readRegisters();

    }

    void afterSyscall() {
        std::cout << "after 'open' syscall\n";
    }

    static Plugin* create() {
        std::cout << "'open' create\n";
        return new OpenPlugin();
    }
};

REGISTER_PLUGIN(open, OpenPlugin::create);