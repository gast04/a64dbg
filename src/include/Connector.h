#include <unistd.h>
#include <linux/uio.h>
#include <linux/elf.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

class Connector {
private:
    pid_t tracee;
    uint64_t private_memory;

    enum class HwFeature { Watchpoint, Breakpoint };
    bool hw_bp_supported;
    bool hw_watch_supported;

    Connector(){}
    ~Connector(){}
    bool checkHwFeature(HwFeature feature);

public:
    static Connector& getInstance() {
        static Connector instance;
        return instance;
    }

    void init(pid_t tracee_pid);
    void setTracee(pid_t tracee_pid);
    bool attach();
    bool checkHwBpSupport();
    bool checkHwWatchSupport();

    struct user_pt_regs getRegisters();
    void setRegisters(struct user_pt_regs regs);
    size_t readMemory(void* addr, uint8_t* buffer, size_t size);
    size_t writeMemory(void* addr, uint8_t* buffer, size_t size);
    uint32_t getDWORD(void* addr);
    uint64_t getTraceePid();

    uint64_t getPrivateMemory();
    int doSingleStep();
    uint64_t allocateMemoryInChild();
    uint64_t mprotectMemory(uint64_t mem_addr, uint64_t mem_size, uint64_t mem_prot);
};
