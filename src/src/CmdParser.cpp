#include <iostream>

#include "CmdParser.h"

bool CmdParser::parseArgs(std::string cmd_str) {
    // reset before new arg parse
    //args.clear();

    auto parts = splitString(cmd_str, ' ');

    // add if debug
    //for (auto p : parts) {
    //    printf("%s\n", p.c_str());
    //}

    // return as string, and let the command parse if it is used as integer
    cmd_args.insert(cmd_args.begin(), ++parts.begin(), parts.end());

    return true;
}

CMD_TYPE CmdParser::getCmd() {

    CMD_TYPE cmd = CMD_TYPE::NONE;

    while (cmd == CMD_TYPE::NONE) {
        printf("a64> ");
        std::string cmd_str;
        std::getline(std::cin, cmd_str);

        // verify and validate command
        if (cmd_str == "next" || cmd_str == "n") {
            cmd = CMD_TYPE::NEXT;
        }
        else if (cmd_str == "cont" || cmd_str == "c") {
            cmd = CMD_TYPE::CONTIN;
        }
        else if (cmd_str == "syscall" || cmd_str == "s") {
            cmd = CMD_TYPE::SYSCALL_CONTIN;
        }
        else if (cmd_str == "regs" || cmd_str == "r") {
            cmd = CMD_TYPE::SHOW_REGS;
        }
        else if (cmd_str == "exit" || cmd_str == "quit" || cmd_str == "q") {
            cmd = CMD_TYPE::EXIT;
        }
        else if (cmd_str.empty() && last_command != CMD_TYPE::NONE) {
            cmd = last_command;
        }

        else if (cmd_str.substr(0, 5) == "break" ||
                 cmd_str.substr(0, 2) == "bp" ||
                 cmd_str[0] == 'b')
        {
            // verify and parse argumets of mprotect
            if (parseArgs(cmd_str)) {
                cmd = CMD_TYPE::SET_BREAKPOINT;
            }
            // else -> error during argument parsing
        }
        // TODO: implement string helper library
        else if (cmd_str.substr(0, 5) == "mprot") {
            // verify and parse argumets of mprotect
            // expected cmd: "mprot <addr> <size> <flags>"
            //  flags: combination of rwx

            if (parseArgs(cmd_str)) {
                cmd = CMD_TYPE::MEM_MPROTECT;
            }
            // else -> error during argument parsing
        }
        else {
            printf("[!] Unknown command: '%s'\n", cmd_str.c_str());
        }
    }

    // TODO: think about a way on parsing commands with arguments?

    last_command = cmd;
    return cmd;
}