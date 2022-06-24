#include <unistd.h>
#include <linux/uio.h>
#include <linux/elf.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

class Connector {
private:
    pid_t tracee;
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
};