

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

uint64_t tracee = 0;
bool strace_mode = false;

void setBreakPoint()
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

void setHwBreakPoint()
{
    auto args = CmdParser::getInstance().getArgs();
    if (args.size() != 2) {
        printf("[!] Invalid amount of arguments for setting hardware breakpoint!\n");
        return;
    }
    uint64_t bp_addr = stringToU64(args[0]);
    uint64_t bp_num  = stringToU64(args[1]);

    // TODO: Verify for valid address range
    if (Connector::getInstance().setHwBreakpoint(bp_addr, bp_num)) {
        printf("[+] Set Hardware Breakpoint 0x%lx\n", bp_addr);
    }
}

void readTraceeMemory() {
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

void Continue()
{
    if (ptrace(PTRACE_CONT, tracee, 0, 0) < 0) {
        printf("[!] Error in PTRACE_CONT\n");
        return;
    }

    waitpid(tracee, &wait_status, 0);
    if (WIFSTOPPED(wait_status)) {
        printf("[+] PTRACE_CONT stop signal: %d\n", WSTOPSIG(wait_status));
        if( (WSTOPSIG(wait_status)) == 5 )
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
                printf("Checking for hardware breakpoint...\n");

                struct user_hwdebug_state hregs =
                    Connector::getInstance().getHwBreakpoints();

                for (int i = 0; i < Connector::getInstance().getHwBpCount(); ++i) {
                    if (regs.pc == hregs.dbg_regs[i].addr) {
                        // clear HW breakpoint
                        printf("[*] clear hardware breakpoint at pos: %d\n", i);
                        Connector::getInstance().clearHwBreakpoint(i);
                    }
                }
            }
        }
        else {
            printf("[*] Continue, Stop Signal: %d\n", (WSTOPSIG(wait_status)));
            return;
        }
    }
    else {
        printf("[!] Process not stopped by a signal\n");
        return;
    }
}

void syscallContinue() {

    do {
    static bool syscall_enter = true;

    //printf("sysall on: %d\n", tracee);
    if (ptrace(PTRACE_SYSCALL, tracee, 0, 0) < 0) {
        printf("[!] Error in PTRACE_SYSCALL\n");
        return;
    }

    waitpid(tracee, &wait_status, 0);
    if (WIFSTOPPED(wait_status)) {
        if( (WSTOPSIG(wait_status)) == 5 ) // trap signal
        {
            struct user_pt_regs regs = Connector::getInstance().getRegisters();

            // skip following syscalls, those spam a lot and are not
            // very intersting
            uint64_t syscall_num = regs.regs[8];
            if (syscall_num == 98 ||         // futex
                syscall_num == 124 ||        // sched_yield
                syscall_num == 226 ||        // mprotect
                syscall_num == 233           // madvise
            ) {
                continue;
            }

            auto t = aarch64_syscalls[syscall_num];
            if (t.name != nullptr) {

                printf("[!] (%ld) Syscall %c %s(%llu) at 0x%llx, \n",
                    tracee,
                    syscall_enter ? 'E' : 'X',
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

            // on return from clone we need to follow the child
            if (CmdParser::getInstance().follow_fork && syscall_num == 220 && syscall_enter) {
                uint64_t msg = 0;
                ptrace(PTRACE_GETEVENTMSG, tracee, NULL, &msg);
                //printf("ret msg: %lu\n", msg);
                tracee = msg;
                Connector::getInstance().setTracee(msg);
            }

            // TODO: how to distinguish between enter/exit?
            // probably no way if manual input is processed
        }
        else {
            printf("[*] Syscall, Stop Signal: %d\n", (WSTOPSIG(wait_status)));
            return;
        }
    }
    else {
        printf("[!] Process not stopped by a signal\n");
        return;
    }
    } while(strace_mode);
}


void mprotMem() {

    // get extra mprot args
    auto args = CmdParser::getInstance().getArgs();
    if (args.size() != 3) {
        printf("[!] Invalid amount of arguments for mprot!\n");
        printf("    mprot <addr> <size> <prot_flags>\n");
        return;
    }

    uint64_t mem_addr = stringToU64(args[0]);
    uint64_t mem_size = stringToU64(args[1]);
    uint64_t mem_prot = stringToU64(args[2]);
    if (mem_addr == -1 || mem_size == -1 || mem_prot == -1) {
        printf("[!] Check args for mprot!\n");
        return;
    }

    printf("[*] mprot parsed args: %lu, %lu, %lu\n",
           mem_addr, mem_size, mem_prot);

    int result = Connector::getInstance().mprotectMemory(
                    mem_addr, mem_size, mem_prot);

    printf("[+] mprot result: 0x%x\n", result);
}

void issueCommand(CMD_TYPE command)
{
    switch (command)
    {
    case CMD_TYPE::EXIT:
        exit(0); break;
    case CMD_TYPE::NEXT:
        wait_status = Connector::getInstance().doSingleStep();
        break;
    case CMD_TYPE::CONTIN:
        Continue(); break;
    case CMD_TYPE::SHOW_REGS:
        printRegsMap(); break;
    case CMD_TYPE::MEM_MPROTECT:
        mprotMem(); break;
    case CMD_TYPE::MEM_MMAP:
        Connector::getInstance().allocateMemoryInChild();
        break;
    case CMD_TYPE::SYSCALL_CONTIN:
        syscallContinue(); break;
    case CMD_TYPE::SET_BREAKPOINT:
        setBreakPoint(); break;
    case CMD_TYPE::SET_HW_BREAKPOINT:
        setHwBreakPoint(); break;
    case CMD_TYPE::READ_MEMORY:
        readTraceeMemory(); break;
    case CMD_TYPE::STRACE_MODE:
        strace_mode = true;
        syscallContinue(); break;
    case CMD_TYPE::HELP:
        CmdParser::getInstance().printHelpMessage(); break;
    default:
        // should never happen
        printf("Not implemented command!");
        break;
    }
}


int main(int argc, char** argv) {

    auto& cmdparser = CmdParser::getInstance();
    if (!cmdparser.startUpArgs(argc, argv)) {
        return -1;
    }

    if (!plugin_helper.checkForPlugins()) {
        printf("[Error] Invalid Plugins detected!\n");
        return -1;
    }

    bool MODE_ATTACH = cmdparser.attach_mode;
    tracee = cmdparser.target_pid;

    if (!MODE_ATTACH) {
        tracee = fork();
        if (tracee == 0) {
            execute_debugee(tracee, cmdparser.binary_name, argv);
            printf("[ERROR] Child is supposed to never return!\n");
            return 0;
        }
    }

    // init connector, executes ptrace commands
    auto& connector = Connector::getInstance();
    connector.init(tracee);

    if (MODE_ATTACH) {
        if (!connector.attach()) {
            printf("[!] Could not attach to: %lu!\n", tracee);
            return -1;
        }
    }

    // when not attaching the new process is calling traceme
    printf("[*] Debugger Started, tracee pid: %lu\n", tracee);
    waitpid(tracee, &wait_status, 0);

    if (cmdparser.follow_fork) {
        printf("[*] Enable follow fork\n");
        uint32_t popts = /*PTRACE_O_TRACESYSGOOD*/
             PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK; // | PTRACE_O_TRACEVFORK | PTRACE_O_TRACEFORK;
        int res = ptrace(PTRACE_SETOPTIONS, tracee, 0, (unsigned long)popts);
        printf("set ptrace options: %d\n", res);
    }

    // allocate a private memory region inside the tracee for page based
    // breakpoints
    //connector.allocateMemoryInChild();

    // check hardware support
    connector.checkHwBpSupport();
    connector.checkHwWatchSupport();

    CMD_TYPE command = CMD_TYPE::NONE;

    // debugger input loop
    while(WIFSTOPPED(wait_status)) {
        if (command != CMD_TYPE::SHOW_REGS &&
            command != CMD_TYPE::SET_BREAKPOINT &&
            command != CMD_TYPE::SET_HW_BREAKPOINT &&
            command != CMD_TYPE::READ_MEMORY &&
            command != CMD_TYPE::MEM_MPROTECT)
        {
            printCmdHeader();
        }
        command = cmdparser.getCmd();
        issueCommand(command);
    }

    printf("Tracee done\n");
    return 0;
}
