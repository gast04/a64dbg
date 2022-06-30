#include <unistd.h>
#include <linux/uio.h>
#include <linux/elf.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

class Connector {
private:
    pid_t tracee;
    uint64_t private_memory;

    Connector(){}
    ~Connector(){}

public:
    static Connector& getInstance() {
        static Connector instance;
        return instance;
    }

    void init(pid_t tracee_pid);
    bool attach();
    struct user_pt_regs getRegisters();
    void setRegisters(struct user_pt_regs regs);
    size_t readMemory(void* addr, uint8_t* buffer, size_t size);
    size_t writeMemory(void* addr, uint8_t* buffer, size_t size);

    uint64_t getPrivateMemory();
    uint64_t allocateMemoryInChild();
    bool mprotectMemory(uint64_t mem_addr, uint64_t mem_size, uint64_t mem_prot);
};
