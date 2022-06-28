/*
    https://dxuuu.xyz/cpp-static-registration.html#function-1
*/

#pragma once

#include <unordered_map>
#include <functional>

#include <sys/ptrace.h> // needed for registers type


class Plugin {
    public:
    virtual void beforeSyscall() = 0;
    virtual void afterSyscall() = 0;
};

class PluginRegistry {
public:
    using PluginCreateFunction = std::function<Plugin*()>;
    using PluginMap = std::unordered_map<std::string, PluginCreateFunction>;

    static bool add(const std::string& name, PluginCreateFunction pcf) {
        auto& map = getPluginMap();
        if (map.find(name) != map.end()) {
            // same named plugins alreay fail at compile time, due to the
            // _entry boolean, so no name clash check needed
            return false;
        }

        // add plugin to map for later creation
        map[name] = pcf;
        return true;
    }

    static Plugin* create(const std::string& name) {
        auto& map = getPluginMap();
        if (map.find(name) == map.end()) {
            std::cout << "[!] trying to create a plugin which has not been";
            std::cout << "    registerd: '" << name << "'" << std::endl;

            // maybe crashing here makes sense?
            return nullptr;
        }
        return map[name]();
    }

    static const PluginMap& getMap() {
        return getPluginMap();
    }

private:
    // Use Meyer's singleton to prevent SIOF
    static PluginMap& getPluginMap() {
        static PluginMap map;
        return map;
    }
};

#define REGISTER_PLUGIN(plugin_name, create_func) \
    bool plugin_name##_entry = PluginRegistry::add(#plugin_name, (create_func))


// #############################################################################
// Plugin Helper Functions

struct user_pt_regs readRegisters();
void writeRegisters(struct user_pt_regs regs);
