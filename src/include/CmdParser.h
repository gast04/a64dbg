#include <string>
#include <vector>

#include "Utils.h"

enum CMD_TYPE {
    NONE,
    EXIT,
    NEXT,
    CONTIN,
    SHOW_REGS,
    MEM_MPROTECT,
    SYSCALL_CONTIN,
    SET_BREAKPOINT,
};

class CmdParser
{
private:
    CMD_TYPE last_command = CMD_TYPE::NONE;
    std::vector<std::string> cmd_args;

    CmdParser() {}
    ~CmdParser() {}
    bool parseArgs(std::string cmd_str);

public:
    static CmdParser& getInstance() {
        static CmdParser instance;
        return instance;
    }

    CMD_TYPE getCmd();
    std::vector<std::string> getArgs() { return cmd_args; }
};
