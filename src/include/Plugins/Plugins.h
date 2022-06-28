#pragma once

#include <map>
#include <unordered_map>
#include <functional>

#include <sys/ptrace.h> // needed for registers type

class Plugin {
    public:
    virtual void beforeSyscall() {};
    virtual void afterSyscall() {};
    static Plugin* create() {
        std::cout << "[*] Parent Plugin Create\n";
        return nullptr;
    }
};

template <typename T>
class PluginRegistry {
public:
    typedef std::function<T*()> FactoryFunction;
    typedef std::unordered_map<std::string, FactoryFunction> FactoryMap;

    static bool add(const std::string& name, FactoryFunction fac) {
        auto map = getFactoryMap();
        if (map.find(name) != map.end()) {
            return false;
        }

        getFactoryMap()[name] = fac;
        return true;
    }

    static T* create(const std::string& name) {
        auto map = getFactoryMap();
        if (map.find(name) == map.end()) {
            return nullptr;
        }

        return map[name]();
    }

    static const FactoryMap& getMap() {
        return getFactoryMap();
    }

private:
    // Use Meyer's singleton to prevent SIOF
    static FactoryMap& getFactoryMap() {
        static FactoryMap map;
        return map;
    }
};

#define REGISTER_PLUGIN(plugin_name, create_func) \
    bool plugin_name ## _entry = PluginRegistry<Plugin>::add(#plugin_name, (create_func))


// #############################################################################
// Plugin Helper Functions

struct user_pt_regs readRegisters();
void writeRegisters(struct user_pt_regs regs);
