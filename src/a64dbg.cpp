

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <linux/elf.h>
#include <linux/uio.h>
//#include <sys/personality.h>

#include <stdio.h>
#include <signal.h>
#include <syscall.h>
#include <errno.h>

#include "Connector.h"
#include "CmdParser.h"


int wait_status;
std::map <uint64_t, uint64_t> BreakPoints;


void CheckDWORDMem(pid_t child, uint64_t addr)
{
    //unsigned addr = 0x8048096;
    unsigned data = ptrace(PTRACE_PEEKTEXT, child, (void*)addr, 0);
    printf("[+] Checking BP Location 0x%08lx: 0x%08x\n", addr, data);
}

void ListBreakPoints()
{
    // Printing values in the map
    std::cout << std::endl << "######### Listing  BreakPoints #########" << std::endl;  
    for( std::map< uint64_t, uint64_t >::iterator iter=BreakPoints.begin(); iter!=BreakPoints.end(); ++iter)  
    {
        //std::cout << (*iter).first << ": " << (*iter).second << std::endl;  
        printf("[+] Listing BPs 0x%08lx: 0x%08lx\n", iter->first, iter->second);
    }

    std::cout << std::endl << "######### Finished BreakPoints #########\n\n" << std::endl;
}


void SetBreakPoint(pid_t child, uint64_t addr)
{
    /* Write the trap instruction 'int 3' into the address */
    unsigned data = ptrace(PTRACE_PEEKTEXT, child, (void*)addr, 0);
    unsigned data_with_trap = (data & 0xFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT, child, (void*)addr, (void*)data_with_trap);
    printf("[+] Setting BP 0x%08lx: 0x%08x\n", addr, data);

    BreakPoints.insert ( std::pair< uint64_t, uint64_t>(addr,data) );
}


void execute_debugee(pid_t child, const std::string& prog_name, char** argv)
{
    //personality(ADDR_NO_RANDOMIZE); // Disable ALSR

    if (ptrace(PTRACE_TRACEME) < 0) {
        std::cerr << "[!] Error in PTRACE_TRACEME\n";
        return;
    }
    argv[0] = nullptr;
    execvp(prog_name.c_str(), argv);
}

void singleStep(pid_t child)
{
    if (ptrace(PTRACE_SINGLESTEP, child, 0, 0) < 0) {
        printf("[!] Error in PTRACE_SINGLESTEP\n");
        return;
    }

    wait(&wait_status);
}


void Continue (pid_t child)
{
    struct user_regs_struct regs;

    ptrace(PTRACE_CONT, child, 0, 0);
    wait(&wait_status);
    if (WIFSTOPPED(wait_status)) {
        //printf("Child got a signal: %d\n", (WSTOPSIG(wait_status))  );
        if(  (WSTOPSIG(wait_status))  == 5 )
        {
            //ptrace(PTRACE_GETREGS, child, 0, &regs);
            printf("[!] BreakPoint Hit at 0x%08lx \n", regs.pc );

            // Restore data at breakpoint
            std::map<uint64_t , uint64_t>::iterator iter = BreakPoints.find(regs.pc - 1);
            if (iter != BreakPoints.end())
            {
            //std::cout << std::endl<< "[+] Value  => " << BreakPoints.find(regs.pc -= 1)->second << '\n';
            printf("[+] Address 0x%08lx: Data 0x%08lx\n",
                iter->first, iter->second);
            }

            uint64_t addr = BreakPoints.find(regs.pc - 1)->first;
            uint64_t data = BreakPoints.find(regs.pc - 1)->second;

            ptrace(PTRACE_POKETEXT, child, (void*)addr, (void*)data);
            regs.pc -= 1;
            //ptrace(PTRACE_SETREGS, child, 0, &regs);

            // End restore data
        }
    }
    else {
        perror("[-] wait");
        return;
    }

    printf("[+] Child stopped at pc = 0x%08lx\n", regs.pc);
}

void printCmdHeader(pid_t tracee) {
    struct user_pt_regs regs;
    struct iovec io;
    io.iov_base = &regs;
    io.iov_len = sizeof(regs);

    ptrace(PTRACE_GETREGSET, tracee, (void*)NT_PRSTATUS, (void*)&io);
    uint32_t instr = ptrace(PTRACE_PEEKTEXT, tracee, regs.pc, 0);

    printf("[+] (%d) pc: 0x%08lx sp: 0x%08lx instr: 0x%04x\n",
            tracee, regs.pc, regs.sp, instr);
}

void printRegsMap(pid_t tracee) {
    struct user_pt_regs regs;
    struct iovec io;
    io.iov_base = &regs;
    io.iov_len = sizeof(regs);

    ptrace(PTRACE_GETREGSET, tracee, (void*)NT_PRSTATUS, (void*)&io);
    // todo error checking

    printf("----Register Map-------------------------------------------\n");
    for (int i = 0; i < 10; ++i) {
        printf("  R%d:   0x%016lx   R%d:  0x%016lx  R%d:  0x%016lx \n",
            i, regs.regs[i], i+10, regs.regs[i+10], i+20, regs.regs[i+20]);
    }
    printf("  R%d:  0x%016lx\n", 30, regs.regs[30]);
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

    unsigned icounter = 0;

    Connector connector = Connector::getInstance();
    CmdParser cmdparser = CmdParser::getInstance();

    std::string target_binary = argv[1];
    std::cout << "[*] Executable path: " << target_binary << std::endl;

    pid_t tracee = fork();
    if (tracee == 0) {
        execute_debugee(tracee, target_binary, argv);
        printf("Child is supposed to never return!!!");
        return 0;
    }

    // attaching to target process
    /*
    if (!connector.attach(tracee)) {
        printf("Failed to attach!");
        return -1;
    }
    */


    // when not attaching the new process is calling traceme
    printf("[*] Debugger Started, tracee pid: %d\n", tracee);
    wait(&wait_status);

    CMD_TYPE command = CMD_TYPE::NONE;

    // debugger input loop
    while(WIFSTOPPED(wait_status)) {
        if (command != CMD_TYPE::SHOW_REGS) {
            printCmdHeader(tracee);
        }
        command = cmdparser.getCmd();
        issueCommand(tracee, command);
    }
    printf("The child executed %u instructions\n", icounter);
}
