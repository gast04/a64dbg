#include <iostream>

#include "Plugins/Plugins.h"
#include "Plugins/PUtils.h"
#include "Utils/Utils.h"

class TemplatePlugin : public Plugin
{
public:

    void beforeSyscall() override {
        std::cout << "before 'template' syscall\n";
    }

    void afterSyscall() override {
        std::cout << "after 'template' syscall\n";
    }

    static Plugin* create() {
        return new TemplatePlugin();
    }
};

REGISTER_PLUGIN(template, TemplatePlugin::create);