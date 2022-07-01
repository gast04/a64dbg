#include <string>
#include <vector>

#include "Utils/Utils.h"

enum CMD_TYPE {
    NONE,
    EXIT,
    NEXT,
    CONTIN,
    SHOW_REGS,
    MEM_MPROTECT,
    MEM_MMAP,
    SYSCALL_CONTIN,
    SET_BREAKPOINT,
    READ_MEMORY,
    STRACE_MODE,
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

    bool follow_fork;
    bool attach_mode;
    uint64_t target_pid;
    std::string binary_name;
    bool startUpArgs(int argc, char** argv);

    CMD_TYPE getCmd();
    std::vector<std::string> getArgs() { return cmd_args; }
};
