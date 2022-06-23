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
};

class CmdParser {
    CmdParser() {}

    CMD_TYPE last_command = CMD_TYPE::NONE;
    std::vector<std::string> args;

    bool parseMprotArgs(std::string cmd_str);

public:
    static CmdParser& getInstance()
    {
        static CmdParser instance;
        return instance;
    }

    CMD_TYPE getCmd();
    std::vector<std::string> getArgs() { return args; }
};
