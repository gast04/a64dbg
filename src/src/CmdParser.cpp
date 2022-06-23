#include <iostream>

#include "CmdParser.h"

bool CmdParser::parseMprotArgs(std::string cmd_str) {
    // expected cmd: "mprot <addr> <size> <flags>"
    //  flags: combination of rwx

    auto parts = splitString(cmd_str, ' ');

    for (auto p : parts) {
        printf("%s\n", p.c_str());
    }

    // check first args if integer
    //parts[1];

    // check second args if integer
    //parts[2];

    // check third args combination of rwx
    //parts[3];

    // all checks skipped for now
    args = {parts[1], parts[2], parts[3]};


    printf("leaving parser\n");
    return true;
}

CMD_TYPE CmdParser::getCmd() {

    args.clear(); // always reset arguments
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
        else if (cmd_str == "regs" || cmd_str == "r") {
            cmd = CMD_TYPE::SHOW_REGS;
        }
        else if (cmd_str == "exit" || cmd_str == "quit" || cmd_str == "q") {
            cmd = CMD_TYPE::EXIT;
        }
        else if (cmd_str.empty() && last_command != CMD_TYPE::NONE) {
            cmd = last_command;
        }

        // TODO: implement string helper library
        else if (cmd_str.substr(0, 5) == "mprot") {
            // verify and parse argumets of mprotect
            if (parseMprotArgs(cmd_str)) {
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