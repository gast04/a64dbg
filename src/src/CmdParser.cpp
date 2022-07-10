#include <iostream>
#include <vector>
#include <unordered_map>

#include "CmdParser.h"

bool CmdParser::parseArgs(std::string cmd_str) {
    // reset before new arg parse
    cmd_args.clear();

    auto parts = splitString(cmd_str, ' ');

    // add if debug
    //for (auto p : parts) {
    //    printf("%s\n", p.c_str());
    //}

    // return as string, and let the command parse if it is used as integer
    cmd_args.insert(cmd_args.begin(), ++parts.begin(), parts.end());

    return true;
}


typedef struct cmd_struct {
    std::vector<std::string> cmd;
    CMD_TYPE type;
    int args_num;
    std::string info_msg;
} CMD_STRUCT;


std::unordered_map<std::string, cmd_struct> commands = {
    {"exit", {
        {"q", "quit", "exit"},
        CMD_TYPE::EXIT,
        0,
        "end debugging session"
    }},
    {"help", {
        {"?", "help"},
        CMD_TYPE::HELP,
        0,
        "print help information"
    }},
    {"next", {
        {"n", "next"},
        CMD_TYPE::NEXT,
        0,
        "single step instruction"
    }},
    {"continue", {
        {"c", "cont", "continue"},
        CMD_TYPE::CONTIN,
        0,
        "single step instruction"
    }},
    {"breakpoint", {
        {"bp", "breakpoint"},
        CMD_TYPE::SET_BREAKPOINT,
        1,
        "set software breakpoint, bp <addr>"
    }},
    {"hw_breakpoint", {
        {"hbp", "hbreakpoint"},
        CMD_TYPE::SET_HW_BREAKPOINT,
        2,
        "set hardware breakpoint, hbp <addr> <reg_num>"
    }},
    {"read_memory", {
        {"rm", "readm"},
        CMD_TYPE::READ_MEMORY,
        2,
        "read tracee memory, readm <addr> <size>"
    }},
    {"read_registers", {
        {"re", "regs"},
        CMD_TYPE::SHOW_REGS,
        0,
        "show registers"
    }},
    {"mprot", {
        {"mprot"},
        CMD_TYPE::MEM_MPROTECT,
        3,
        "change tracee memory protection, mprot <addr> <size> <flags>"
    }},
    {"mmap", {
        {"mmap"},
        CMD_TYPE::MEM_MMAP,
        3,
        "mmap memory in tracee, mmap <addr> <size> <flags>"
    }},
};

CMD_TYPE CmdParser::getCmd() {

    while (true) {
        printf("a64> ");
        std::string cmd_str;
        std::getline(std::cin, cmd_str);

        if (cmd_str.empty() && last_command != CMD_TYPE::NONE) {
            return last_command;
        }

        // parse command
        for (auto& c : commands) {
            CMD_STRUCT& cs = commands[c.first];

            bool has_match = false;
            for (auto& sc : cs.cmd) {
                has_match |= cmd_str.substr(0, sc.size()) == sc;
            }

            if (!has_match)
                continue;

            if (cs.args_num != 0) {
                if (parseArgs(cmd_str)) {
                    last_command = cs.type;
                    return cs.type;
                }
            }
            else {
                last_command = cs.type;
                return cs.type;
            }
        }

        printf("[!] Unknown command: '%s'\n", cmd_str.c_str());
    }

    // never reaches this point
    return CMD_TYPE::NONE;
}

void CmdParser::printHelpMessage() {

    printf("available commands:\n\n");
    for (auto& c : commands) {
        CMD_STRUCT& cs = commands[c.first];

        printf("  ");
        for(auto& cs : cs.cmd)
            printf("%s ", cs.c_str());
        printf("(%d)\n", cs.args_num);
        printf("      %s\n", cs.info_msg.c_str());
    }
    printf("\n");
}

bool CmdParser::startUpArgs(int argc, char** argv) {

    if (argc == 1) {
        // print help
        printf("Usage: a64dbg [-f] -p <pid>|<binary>\n");
        printf("    -f          follow fork/clone\n");
        printf("    -p <pid>    pid to attach\n");
        printf("    <binary>    program to start\n");
        return false;
    }

    follow_fork = false;
    attach_mode = false;
    target_pid = 0;

    try {
        for(int i = 1; i < argc; ++i) {
            std::string param = argv[i];

            if (param == "-f") {
                follow_fork = true;
            }
            else if (param == "-p") {
                attach_mode = true;

                if (i+1 >= argc) {
                    printf("No argument found for <pid>!\n");
                    return false;
                }

                target_pid = stringToU64(argv[i+1]);
                if (target_pid == -1) {
                    printf("Could not parse <pid>!\n");
                    return false;
                }
                i++;
            }
            else {
                binary_name = param;
            }
        }
    }
    catch (void* e) {
        printf("[!] Error during Argument parsing\n");
        return false;
    }

    /*
    printf("Args Summary:\n");
    printf("    binary name: %s\n", binary_name.c_str());
    printf("    attach mode: %d\n", attach_mode);
    printf("    follow fork: %d\n", follow_fork);
    printf("    target pid:  %d\n", target_pid);
    */

    if (attach_mode && !binary_name.empty()) {
        printf("[Parser] Pid and binary given, ignoring binary!\n");
        binary_name = "";
    }

    if (follow_fork)
        printf("[*] Follow Fork ENABLED\n");

    return true;
}