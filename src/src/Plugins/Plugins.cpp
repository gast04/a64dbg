#include <iostream>
#include <string.h>

#include "Plugins/Plugins.h"

bool PluginHelper::checkForPlugins() {

    // bruh, fix me
    #include "aarch64/Syscalls.h"

    printf("[*] Checking for enabled Plugins...\n");
    auto plugin_map = PluginRegistry::getMap();

    for (auto& p : plugin_map) {
        std::cout << "[*] Init Plugin: " << p.first << std::endl;

        bool is_syscall = false;
        for (int i = 0; i < MAX_SYSCALL_NUM; ++i) {

            // some syscall numbers are specific and have not been added
            if (aarch64_syscalls[i].name == nullptr)
                continue;

            //printf("%s\n", aarch64_syscalls[i].name);
            if (strncmp(p.first.c_str(), aarch64_syscalls[i].name,
                        p.first.size()) == 0)
            {
                is_syscall = true;
                break;
            }
        }

        if (!is_syscall) {
            printf("[!] Enabled plugin is not named like a known syscall! ");
            printf("'%s'\n", p.first.c_str());
            return false;
        }

        registered_plugins[p.first] = PluginRegistry::create(p.first);
    }

    return true;
}

void PluginHelper::callBefore(const std::string& plugin_name) {
    if (registered_plugins.find(plugin_name) == registered_plugins.end()) {
        return;
    }
    registered_plugins[plugin_name]->beforeSyscall();
}

void PluginHelper::callAfter(const std::string& plugin_name) {
    if (registered_plugins.find(plugin_name) == registered_plugins.end()) {
        return;
    }
    registered_plugins[plugin_name]->afterSyscall();
}
