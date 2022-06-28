#include <iostream>

#include "Plugins/Plugins.h"

class ClosePlugin : public Plugin
{
public:

    void beforeSyscall() override {
        std::cout << "before 'close' syscall\n";
    }

    void afterSyscall() override {
        std::cout << "after 'close' syscall\n";
    }

    static Plugin* create() {
        return new ClosePlugin();
    }
};

REGISTER_PLUGIN(close, ClosePlugin::create);