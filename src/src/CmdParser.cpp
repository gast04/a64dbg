#include <iostream>

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
        else if (cmd_str == "strace") {
            cmd = CMD_TYPE::STRACE_MODE;
        }
        else if (cmd_str == "syscall" || cmd_str == "s") {
            cmd = CMD_TYPE::SYSCALL_CONTIN;
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
            // expected cmd: "break <addr>"

            if (parseArgs(cmd_str)) {
                cmd = CMD_TYPE::SET_BREAKPOINT;
            }
        }
        else if (cmd_str.substr(0, 5) == "readm" ||
                 cmd_str.substr(0, 2) == "rm")
        {
            // expected cmd: "readm <addr> <size>"

            if (parseArgs(cmd_str)) {
                cmd = CMD_TYPE::READ_MEMORY;
            }
        }
        // careful by only checking for 'r'
        else if (cmd_str == "regs" || cmd_str == "r") {
            cmd = CMD_TYPE::SHOW_REGS;
        }
        else if (cmd_str.substr(0, 5) == "mprot") {
            // verify and parse argumets of mprotect
            // expected cmd: "mprot <addr> <size> <flags>"
            //  flags: combination of rwx

            if (parseArgs(cmd_str)) {
                cmd = CMD_TYPE::MEM_MPROTECT;
            }
            // else -> error during argument parsing
        }
        else if (cmd_str.substr(0, 4) == "mmap") {
            // verify and parse argumets of mprotect
            // expected cmd: "mmap <addr> <size> <flags>"
            //  flags: combination of rwx

            cmd = CMD_TYPE::MEM_MMAP;
            /*if (parseArgs(cmd_str)) {
                cmd = CMD_TYPE::MEM_MPROTECT;
            }*/
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