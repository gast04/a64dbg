

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>

#include <sys/ptrace.h> // TODO: remove
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <linux/elf.h>  // TODO: remove
#include <linux/uio.h>  // TODO: remove
#include <sys/personality.h>

#include <stdio.h>
#include <signal.h>
#include <syscall.h>
#include <errno.h>

#include "Connector.h"
#include "CmdParser.h"
#include "Utils/Memory.h"
#include "Plugins/Plugins.h"
#include "aarch64/Syscalls.h"


int wait_status;
std::map <uint64_t, uint64_t> BreakPoints; // addr, instruction
PluginHelper plugin_helper;

void CheckDWORDMem(pid_t child, uint64_t addr)
{
    //unsigned addr = 0x8048096;
    unsigned data = ptrace(PTRACE_PEEKTEXT, child, (void*)addr, 0);
    printf("[+] Checking BP Location 0x%08lx: 0x%08x\n", addr, data);
}

void setBreakPoint(pid_t tracee)
{
    auto args = CmdParser::getInstance().getArgs();
    if (args.size() != 1) {
        printf("[!] Invalid amount of arguments for setting breakpoint!\n");
        return;
    }
    uint64_t bp_addr = stringToU64(args[0]);

    // TODO: Verify for valid address range

    // write the 'break #0' instruction, 00 00 20 D4
    uint64_t data = ptrace(PTRACE_PEEKTEXT, tracee, (void*)bp_addr, 0);
    uint64_t aarch464_trap = 0xD4200000;
    ptrace(PTRACE_POKETEXT, tracee, (void*)bp_addr, (void*)aarch464_trap);
    printf("[+] Set Breakpoint 0x%lx: 0x%08x\n", bp_addr, (uint32_t)data);
    BreakPoints.insert ( std::pair< uint64_t, uint64_t>(bp_addr
    , data) );
}

void readTraceeMemory(pid_t tracee) {
    auto args = CmdParser::getInstance().getArgs();
    if (args.size() != 2) {
        printf("[!] Invalid amount of arguments for reading memory!\n");
        return;
    }

    // read byte granulariy
    uint64_t r_addr = stringToU64(args[0]);
    uint64_t r_size = stringToU64(args[1]);

    uint32_t* storage = new uint32_t[r_size];
    readProcMemory(tracee, r_addr, (uint8_t*)storage, r_size*4);

    printf("%016lx: ", r_addr);
    for(int i = 0; i < r_size; ++i) {
        printf("%08x ", storage[i]);
        if ((i%5) == 4) {
            if (i == r_size -1) {
                printf("\n");
            }
            else {
                printf("\n%016lx: ", r_addr+=20);
            }
        }
    }
    printf("\n");
    delete[] storage;
}

void execute_debugee(pid_t child, const std::string& prog_name, char** argv)
{
    personality(ADDR_NO_RANDOMIZE); // Disable ASLR

    if (ptrace(PTRACE_TRACEME) < 0) {
        printf("[!] Error in PTRACE_TRACEME\n");
        return;
    }
    argv[0] = nullptr;
    execvp(prog_name.c_str(), argv);
}

void singleStep(pid_t tracee)
{
    if (ptrace(PTRACE_SINGLESTEP, tracee, 0, 0) < 0) {
        printf("[!] Error in PTRACE_SINGLESTEP\n");
        return;
    }

    wait(&wait_status);
}

void Continue(pid_t tracee)
{
    if (ptrace(PTRACE_CONT, tracee, 0, 0) < 0) {
        printf("[!] Error in PTRACE_CONT\n");
        return;
    }

    wait(&wait_status);
    if (WIFSTOPPED(wait_status)) {
        printf("Tracee Stop Signal: %d\n", (WSTOPSIG(wait_status)));

        if( (WSTOPSIG(wait_status)) == 5 ) // breakpoint hit
        {
            struct user_pt_regs regs = Connector::getInstance().getRegisters();
            printf("[!] Breakpoint Hit at 0x%llx\n", regs.pc);

            // Restore data at breakpoint
            auto iter = BreakPoints.find(regs.pc);
            if (iter != BreakPoints.end())
            {
                printf("[+] Restore at 0x%016llx - 0x%08lx\n",
                        iter->first, (uint32_t)iter->second);
                ptrace(PTRACE_POKETEXT, tracee, (void*)iter->first, (void*)iter->second);
            }
            else {
                printf("Breakpoint not found in list... hein??\n");
            }
        }
    }
    else {
        printf("[!] wait error\n");
    }

    // printf("[+] Child stopped at pc = 0x%08lx\n", regs.pc);
}

void syscallContinue(pid_t tracee) {

    static bool syscall_enter = true;

    if (ptrace(PTRACE_SYSCALL, tracee, 0, 0) < 0) {
        printf("[!] Error in PTRACE_SYSCALL\n");
        return;
    }

    wait(&wait_status);
    if (WIFSTOPPED(wait_status)) {
        printf("Tracee Stop Signal: %d\n", (WSTOPSIG(wait_status)));

        if( (WSTOPSIG(wait_status)) == 5 ) // trap signal
        {
            struct user_pt_regs regs = Connector::getInstance().getRegisters();

            uint64_t syscall_num = regs.regs[8];

            auto t = aarch64_syscalls[syscall_num];
            if (t.name != nullptr) {
                printf("[!] Syscall %s(%llu) at 0x%llx, \n",
                    t.name, regs.regs[8], regs.pc);

                if(syscall_enter) {
                    plugin_helper.callBefore(t.name);
                    syscall_enter = false;
                }
                else {
                    plugin_helper.callAfter(t.name);
                    syscall_enter = true;
                }

            }
            else {
                printf("[!] Syscall %llu at 0x%llx, \n", regs.regs[8], regs.pc);
            }

            // TODO: how to distinguish between enter/exit?
            // probably no way if manual input is processed
        }
    }
    else {
        printf("[!] wait error\n");
    }
}

void printCmdHeader(pid_t tracee) {
    struct user_pt_regs regs = Connector::getInstance().getRegisters();
    uint32_t instr = ptrace(PTRACE_PEEKTEXT, tracee, regs.pc, 0);
    printf("[+] (%d) pc: 0x%08lx sp: 0x%08lx instr: 0x%04x\n",
            tracee, regs.pc, regs.sp, instr);
}

void printRegsMap(pid_t tracee) {
    struct user_pt_regs regs = Connector::getInstance().getRegisters();

    printf("----Register Map-------------------------------------------\n");
    for (int i = 0; i < 10; ++i) {
        printf("  x%d:   0x%016llx   x%d:  0x%016llx  x%d:  0x%016llx \n",
            i, regs.regs[i], i+10, regs.regs[i+10], i+20, regs.regs[i+20]);
    }
    printf("  x%d:  0x%016llx\n", 30, regs.regs[30]);
    printf("-----------------------------------------------------------\n");
}

void mprotMem(pid_t tracee) {

    // get extra mprot args
    printf("fetching args\n");
    auto args = CmdParser::getInstance().getArgs();
    for (auto p : args) {
        printf("A %s\n", p.c_str());
    }
    uint64_t mem_addr = atol(args[0].c_str());
    uint64_t mem_size = atol(args[1].c_str());

    printf("mprotect args: %lu %lu\n", mem_addr, mem_size);
}

void issueCommand(pid_t tracee , CMD_TYPE command)
{
    switch (command)
    {
    case CMD_TYPE::EXIT:
        exit(0); break;
    case CMD_TYPE::NEXT:
        singleStep(tracee); break;
    case CMD_TYPE::CONTIN:
        Continue(tracee); break;
    case CMD_TYPE::SHOW_REGS:
        printRegsMap(tracee); break;
    case CMD_TYPE::MEM_MPROTECT:
        mprotMem(tracee); break;
    case CMD_TYPE::SYSCALL_CONTIN:
        syscallContinue(tracee); break;
    case CMD_TYPE::SET_BREAKPOINT:
        setBreakPoint(tracee); break;
    case CMD_TYPE::READ_MEMORY:
        readTraceeMemory(tracee); break;
    default:
        // should never happen
        break;
    }
}


int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Usage: ./debugger <binary path>\n");
        return -1;
    }

    std::string target_binary = argv[1];
    std::cout << "[*] Executable path: " << target_binary << std::endl;

    if (!plugin_helper.checkForPlugins()) {
        printf("Invalid Plugins detected! stop a64dbg\n");
        return -1;
    }

    pid_t tracee = fork();
    if (tracee == 0) {
        execute_debugee(tracee, target_binary, argv);
        printf("Child is supposed to never return!!!");
        return 0;
    }

    // init singletons
    auto& connector = Connector::getInstance();
    connector.init(tracee);
    auto& cmdparser = CmdParser::getInstance();

    // attaching to target process, NOT implemented

    // when not attaching the new process is calling traceme
    printf("[*] Debugger Started, tracee pid: %d\n", tracee);
    wait(&wait_status);

    CMD_TYPE command = CMD_TYPE::NONE;

    // debugger input loop
    while(WIFSTOPPED(wait_status)) {
        if (command != CMD_TYPE::SHOW_REGS &&
            command != CMD_TYPE::SET_BREAKPOINT &&
            command != CMD_TYPE::READ_MEMORY)
        {
            printCmdHeader(tracee);
        }
        command = cmdparser.getCmd();
        issueCommand(tracee, command);
    }

    printf("Tracee done\n");
    return 0;
}
